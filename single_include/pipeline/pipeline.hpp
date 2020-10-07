#pragma once
#include <tuple>

namespace pipeline {

template <typename T1, typename T2> class pipe_pair;

namespace details {

// is_tuple constexpr check
template <typename> struct is_tuple : std::false_type {};
template <typename... T> struct is_tuple<std::tuple<T...>> : std::true_type {};

template <typename Test, template <typename...> class Ref>
struct is_specialization : std::false_type {};

template <template <typename...> class Ref, typename... Args>
struct is_specialization<Ref<Args...>, Ref> : std::true_type {};

template <class F, size_t... Is> constexpr auto index_apply_impl(F f, std::index_sequence<Is...>) {
  return f(std::integral_constant<size_t, Is>{}...);
}

template <size_t N, class F> constexpr auto index_apply(F f) {
  return index_apply_impl(f, std::make_index_sequence<N>{});
}

// Unpacks Tuple into a parameter pack
// Calls f(parameter_pack)
template <class Tuple, class F> constexpr auto apply(Tuple t, F f) {
  return index_apply<std::tuple_size<Tuple>{}>([&](auto... Is) { return f(std::get<Is>(t)...); });
}

template <size_t N, typename T> constexpr decltype(auto) make_repeated_tuple(T t) {
  if constexpr (N == 1) {
    return t;
  } else {
    return make_repeated_tuple<N - 1>(std::tuple_cat(std::make_tuple(std::get<0>(t)), t));
  }
}

template<typename T, typename F, int... Is>
void
for_each(T&& t, F f, std::integer_sequence<int, Is...>) {
  auto l = { (f(std::get<Is>(t)), 0)... };
}

template<typename... Ts, typename F>
void
for_each_in_tuple(std::tuple<Ts...> const& t, F f) {
  for_each(t, f, std::make_integer_sequence<int, sizeof...(Ts)>());
}

} // namespace details

} // namespace pipeline
#pragma once
// #include <pipeline/details.hpp>

namespace pipeline {

template <typename Fn> class fn {
  Fn fn_;

public:
  fn(Fn fn) : fn_(fn) {}

  template <typename... T> decltype(auto) operator()(T &&... args) { return fn_(args...); }

  template <typename... A> static constexpr bool is_invocable_on() {
    return std::is_invocable<Fn, A...>::value;
  }

  template <typename T> auto operator|(T &&rhs) {
    return pipe_pair<fn<Fn>, T>(*this, std::forward<T>(rhs));
  }
};

} // namespace pipeline
#pragma once
// #include <pipeline/details.hpp>

namespace pipeline {

template <typename T1, typename T2> class pipe_pair {
  T1 left_;
  T2 right_;

public:
  pipe_pair(T1 left, T2 right) : left_(left), right_(right) {}

  template <typename... T> decltype(auto) operator()(T &&... args) {
    typedef typename std::result_of<T1(T...)>::type left_result_type;
    
    if constexpr (!std::is_same<left_result_type, void>::value) {
      return right_(left_(std::forward<T>(args)...));
    } else {
      left_(std::forward<T>(args)...);
      return right_();
    }
  }

  template <typename T3> auto operator|(T3 &&rhs) {
    return pipe_pair<pipe_pair<T1, T2>, T3>(*this, std::forward<T3>(rhs));
  }
};

template <typename T1, typename T2> auto pipe(T1 &&t1, T2 &&t2) {
  return pipe_pair<T1, T2>(std::forward<T1>(t1), std::forward<T2>(t2));
}

template <typename T1, typename T2, typename... T> auto pipe(T1 &&t1, T2 &&t2, T &&... args) {
  return pipe(pipe<T1, T2>(std::forward<T1>(t1), std::forward<T2>(t2)), std::forward<T>(args)...);
}

} // namespace pipeline
#pragma once
#include <functional>
#include <future>
// #include <pipeline/details.hpp>
// #include <pipeline/fn.hpp>
#include <thread>

namespace pipeline {

template <typename Fn, typename... Fns> class fork_into {
  std::tuple<Fn, Fns...> fns_;

public:
  fork_into(Fn first, Fns... fns) : fns_(first, fns...) {}

  template <typename... Args> decltype(auto) operator()(Args&&... args) {
    typedef typename std::result_of<Fn(Args...)>::type result_type;

    std::vector<std::future<result_type>> futures;

    auto apply_fn = [&futures, args_tuple = std::tuple<Args...>(std::forward<Args>(args)...)](auto fn) {
      auto unpack = [](auto tuple, auto fn) { return details::apply(tuple, fn); };
      futures.push_back(std::async(std::launch::async | std::launch::deferred, unpack, args_tuple, fn));
    };

    details::for_each_in_tuple(fns_, apply_fn);

    if constexpr (std::is_same<result_type, void>::value) {
      for (auto& f: futures) {
        f.get();
      }
    } else {
      std::vector<result_type> results;
      for (auto& f: futures) {
        results.push_back(f.get());
      }
      return results;
    }
  }

  template <typename T3> auto operator|(T3 &&rhs) {
    return pipe_pair<fork_into<Fn, Fns...>, T3>(*this, std::forward<T3>(rhs));
  }
};

} // namespace pipeline
#pragma once
// #include <pipeline/details.hpp>
// #include <pipeline/fn.hpp>
#include <vector>

namespace pipeline {

template <typename Fn>
class for_each {
  Fn fn_;

public:
  for_each(Fn fn) : fn_(fn) {}

  template <typename Container>
  decltype(auto) operator()(Container&& args) {
    typedef typename std::result_of<Fn(typename Container::value_type&)>::type result_type;

    if constexpr (std::is_same<result_type, void>::value) {
      // result type is void
      std::vector<std::future<result_type>> futures;
      for (auto& arg: std::forward<Container>(args)) {
        futures.push_back(std::async(std::launch::async | std::launch::deferred, fn_, arg));
      }

      for (auto& f: futures) {
        f.get();
      }
    }
    else {
      // result is not void
      std::vector<result_type> results;
      std::vector<std::future<result_type>> futures;
      for (auto& arg: std::forward<Container>(args)) {
        futures.push_back(std::async(std::launch::async | std::launch::deferred, fn_, arg));
      }

      for (auto& f: futures) {
        results.push_back(f.get());
      }
      return results;
    }
  }
};  

}
