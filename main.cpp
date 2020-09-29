#include <algorithm>
#include <functional>
#include <iostream>
#include <tuple>
#include <type_traits>
#include <vector>

template <typename T1, typename T2>
class pipe {
  T1 left_;
  T2 right_;

public:
  pipe(T1 left, T2 right) : left_(left), right_(right) {}

  template <typename... T>
  auto operator()(T... args) {
    return right_(left_(args...));
  }

  template <typename T3>
  auto operator|(const T3& rhs) {
    return pipe<pipe<T1, T2>, T3>(*this, rhs);
  }
};

template <typename Fn, typename... Args>
class fn {
  Fn fn_;
  std::tuple<Args...> args_;

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

public:
  fn(Fn fn, Args... args): fn_(fn), args_(args...) {}

  template <typename... T>
  auto operator()(T... left_args) {
    return apply(std::tuple_cat(std::make_tuple(left_args...), args_), fn_);
  }
  
  template <typename Fn2, typename... Args2>
  auto operator|(const fn<Fn2, Args2...>& rhs) {
    return pipe(*this, rhs);
  }
};

int main() {
 
  {
    auto add = fn([](int a, int b) { return a + b; });
    auto square = fn([](int a) { return a * a; });
    auto pretty_print_square = fn([](int result, std::string msg) { std::cout << msg << std::to_string(result); }, std::string{"Result = "});

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

  // auto pipeline = add | square;
  // std::cout << pipeline();

  // task sum([](int a, int b) -> int { return a + b; });
  // std::cout << sum(1, 2) << "\n";

  // task prod([](int a, int b, int c) -> int { return a * b * c; });
  // std::cout << prod(1, 2, 3) << "\n";

  // task greet([]() -> void { std::cout << "Hello World!\n"; });
  // greet();

  // task square([](int a) { return a * a; });

  // pipeline p(sum, square);
  // std::cout << p.get<0>()(5, 6) << "\n";
}