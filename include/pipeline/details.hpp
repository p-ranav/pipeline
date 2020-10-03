#pragma once
#include <algorithm>
#include <functional>
#include <iostream>
#include <numeric>
#include <tuple>
#include <type_traits>
#include <vector>
#include <cstddef>
#include <tuple>
#include <utility>

namespace pipeline {

template <typename Fn>
class fn;

template <typename T1, typename T2>
class pipe_pair;

template <typename Fn, typename... Fns>
class fork;

template <typename Fn, typename... Fns>
class fork_async;

template <typename Fn, typename... Fns>
class unzip_into;

template <typename Fn, typename... Fns>
class unzip_into_async;

namespace details {

// is_tuple constexpr check
template <typename> struct is_tuple: std::false_type {};
template <typename... T> struct is_tuple<std::tuple<T...>>: std::true_type {};

template <typename Test, template <typename...> class Ref>
struct is_specialization : std::false_type {};

template <template <typename...> class Ref, typename... Args>
struct is_specialization<Ref<Args...>, Ref> : std::true_type {};

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

template <size_t N, typename T>
constexpr decltype(auto) make_repeated_tuple(T t) {
  if constexpr (N == 1) {
    return t;
  } else {
    return make_repeated_tuple<N - 1>(std::tuple_cat(std::make_tuple(std::get<0>(t)), t));
  }
}

template <typename F, typename... Args>
constexpr bool is_invocable_on() {
  if constexpr (details::is_specialization<typename std::remove_reference<F>::type, fn>::value) {
    // F is an `fn` type
    return std::remove_reference<F>::type::template is_invocable_on<Args...>();
  }
  else if constexpr (
    details::is_specialization<typename std::remove_reference<F>::type, pipe_pair>::value ||
    details::is_specialization<typename std::remove_reference<F>::type, fork>::value ||
    details::is_specialization<typename std::remove_reference<F>::type, fork_async>::value ||
    details::is_specialization<typename std::remove_reference<F>::type, unzip_into>::value ||
    details::is_specialization<typename std::remove_reference<F>::type, unzip_into_async>::value
  ) {
    return is_invocable_on<typename F::left_type, Args...>();
  }
  else {
    return std::is_invocable<F, Args...>::value;
  }
}

}

}