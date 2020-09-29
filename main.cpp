#include <algorithm>
#include <functional>
#include <iostream>
#include <numeric>
#include <tuple>
#include <type_traits>
#include <vector>

namespace pipeline {

namespace details {

// is_tuple constexpr check
template <typename> struct is_tuple: std::false_type {};
template <typename... T> struct is_tuple<std::tuple<T...>>: std::true_type {};

template <class F, size_t... Is>
constexpr auto index_apply_impl(F f, std::index_sequence<Is...>) {
  return f(std::integral_constant<size_t, Is> {}...);
}

template <size_t N, class F>
constexpr auto index_apply(F f) {
  return  index_apply_impl(f, std::make_index_sequence<N>{});
}

// Unpacks Tuple into a parameter pack
// Calls f(parameter_pack)
template <class Tuple, class F>
constexpr auto apply(Tuple t, F f) {
  return index_apply<std::tuple_size<Tuple>{}>(
    [&](auto... Is) { return f(std::get<Is>(t)...); }
  );
}

}

template <typename T1, typename T2>
class fork;

template <typename T1, typename T2>
class pipe {
  T1 left_;
  T2 right_;

public:
  pipe(T1 left, T2 right) : left_(left), right_(right) {}

  template <typename... T>
  auto operator()(T&&... args) {
    typedef typename std::result_of<T1(T...)>::type left_result;

    // check if left_ result is a tuple
    if constexpr (details::is_tuple<left_result>::value) {
      // left_ result is a tuple
      // check if right_ takes a tuple
      if constexpr (std::is_invocable<typename T2::function_type, left_result>::value) {
        // right_ does take a tuple - it is invocable
        // call right_(left_result)s
        return right_(left_(std::forward<T>(args)...));
      }
      else {
        // right_ does not take a tuple
        // Unpack tuple into a parameter list and call right_(...)
        return details::apply(left_(std::forward<T>(args)...), right_);
      }
    } else {
      return right_(left_(std::forward<T>(args)...));
    }
  }

  template <typename T3>
  auto operator|(const T3& rhs) {
    return pipe<pipe<T1, T2>, T3>(*this, rhs);
  }
};

template <typename T1, typename T2>
class fork {
  T1 left_;
  T2 right_;

public:
  fork(T1 left, T2 right) : left_(left), right_(right) {}

  template <typename... T>
  auto operator()(T&&... args) {
    typedef typename std::result_of<T1(T...)>::type left_result;
    typedef typename std::result_of<T2(T...)>::type right_result;

    if constexpr (details::is_tuple<left_result>::value) {
      if constexpr (details::is_tuple<right_result>::value) {
        // both results are tuples
        // unpack both results and pack as tuple the concat of each result
        return std::tuple_cat(
          left_(std::forward<T>(args)...),
          right_(std::forward<T>(args)...)
        );
      } else {
        // left result alone is a tuple
        return std::tuple_cat(
          left_(std::forward<T>(args)...),
          std::make_tuple(right_(std::forward<T>(args)...))
        );
      }
    }
    else if constexpr (details::is_tuple<right_result>::value) {
      return std::tuple_cat(
        std::make_tuple(left_(std::forward<T>(args)...)),
        right_(std::forward<T>(args)...)
      );
    }
    else {
      return std::make_tuple(left_(std::forward<T>(args)...), right_(std::forward<T>(args)...));
    }
  }

  template <typename T3>
  auto operator|(const T3& rhs) {
    return pipe<fork<T1, T2>, T3>(*this, rhs);
  }

  template <typename T3>
  auto operator&(const T3& rhs) {
    return fork<fork<T1, T2>, T3>(*this, rhs);
  }
};

template <typename Fn, typename... Args>
class fn {
  Fn fn_;
  std::tuple<Args...> args_;

public:
  typedef Fn function_type;

  fn(Fn fn, Args... args): fn_(fn), args_(args...) {}

  template <typename... T>
  auto operator()(T&&... left_args) {
    return details::apply(std::tuple_cat(std::make_tuple(std::forward<T>(left_args)...), args_), fn_);
  }
  
  template <typename Fn2, typename... Args2>
  auto operator|(const fn<Fn2, Args2...>& rhs) {
    return pipe(*this, rhs);
  }

  template <typename T1, typename T2>
  auto operator|(const pipe<T1, T2>& rhs) {
    return pipe(*this, rhs);
  }

  template <typename T1, typename T2>
  auto operator|(const fork<T1, T2>& rhs) {
    return pipe(*this, rhs);
  }

  template <typename Fn2, typename... Args2>
  auto operator&(const fn<Fn2, Args2...>& rhs) {
    return fork(*this, rhs);
  }
};

}

using namespace pipeline;

int main() {
 
  {
    auto add = fn([](int a, int b) { return a + b; });
    auto square = fn([](int a) { return a * a; });
    auto pretty_print_square = fn([](int result, std::string msg = "Result = ") { std::cout << msg << std::to_string(result); });

    auto pipeline = add | square | pretty_print_square;
    pipeline(5, 10);
  }

  std::cout << std::endl;


  {
    auto input = fn([]() { return std::vector<int>{1, 2, 3, 4, 5}; });
    auto square = fn([](const std::vector<int>& input) {
      std::vector<int> result;
      for (auto i : input) result.push_back(i * i);
      return result;
    });
    auto reverse = fn([](const std::vector<int>& input) {
      std::vector<int> result;
      for (auto i = input.rbegin(); i != input.rend(); i++) {
        result.push_back(*i);
      }
      return result;
    });

    auto to_string = fn([](const std::vector<int>& input) {
      std::string result = "{ ";
      for (auto i : input) {
        result += std::to_string(i) + " ";
      }
      result += "}";
      return result;
    });

    auto pipeline = square | reverse | to_string;

    // apply for a single input
    std::cout << pipeline(std::vector<int>{1, 2, 3, 4, 5}) << "\n";

    // apply to a vector of inputs
    std::vector<std::vector<int>> inputs = {
      {1, 2, 3, 4, 5},
      {6, 7, 8, 9, 10},
      {11, 12, 13, 14, 15}
    };
    std::vector<std::string> result{3};
    std::transform(inputs.begin(), inputs.end(), result.begin(), pipeline);
    for (auto r : result) {
      std::cout << r << "\n";
    }
  }

  {
    auto square = fn([](auto n) { 
      std::transform(n.begin(), n.end(), n.begin(), [](auto i) { return i * i; });
      return n;
    });

    auto mean = fn([](auto n) {
      return std::accumulate(n.begin(), n.end(), 0) / n.size();
    });

    auto root = fn([](auto n) {
      return std::sqrt(n);
    });

    auto rms = square | mean | root;
    std::cout << rms(std::vector<int>{2, 4, 9, 10, 12}) << "\n"; // 8.30662
  }

  // Unpack fork result (which is a tuple) 
  // and call next fn
  {
    auto square = fn([](int a) { return a * a; });
    auto cube = fn([](int a) { return a * a * a; });
    auto pipeline = 
      square
      | (square & cube)
      | fn([](auto square_result, auto cube_result) { 
          std::cout << square_result << " " << cube_result << "\n"; 
        });
    pipeline(5);
  }

  // Unpack fork result (which is a tuple) 
  // and call next fn
  {
    auto add_3 = fn([](int a) { return a + 3; });
    auto square = fn([](int a) { return a * a; });
    auto cube = fn([](int a) { return a * a * a; });
    auto pipeline = 
      square
      | (add_3 & square & cube)
      | fn([](auto offset, auto square, auto cube) { 
          std::cout << offset << " " << square << " " << cube << "\n"; 
        });
    pipeline(5);
  }

  // Keep fork result as a tuple
  // and call next fn
  {
    auto square = fn([](int a) { return a * a; });
    auto cube = fn([](int a) { return a * a * a; });
    auto pipeline = 
      square
      | (square & square & cube)
      | fn([](auto packed_result) { 
          std::cout << std::get<0>(packed_result) << " " << std::get<1>(packed_result) << " " << std::get<2>(packed_result)<< "\n"; 
        });
    pipeline(5);
  }

  // Piping a tuple
  {
    auto make_tuple = fn([]() { return std::make_tuple(5, 3.14f, "Hello World"); });
    auto print = fn([](auto int_a, auto float_b, auto str_c) {
      std::cout << int_a << " " << float_b << " " << str_c << "\n";
    });

    auto pipeline = make_tuple | print;
    pipeline();
    print(15, 6.28f, "Goodbye!");
  }
}