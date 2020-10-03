#pragma once
#include <pipeline/details.hpp>
#include <pipeline/fn.hpp>
#include <thread>
#include <future>
#include <functional>

namespace pipeline {

template <typename Fn, typename... Fns>
class unzip_into_async {
  std::tuple<Fn, Fns...> fns_;

  template <class F, class Tuple1, class Tuple2, std::size_t... I>
  auto apply2_impl(F&& f, Tuple1&& t1, Tuple2&& t2, std::index_sequence<I...>) {
    return fork_async((std::forward<F>(f)(std::get<I>(std::forward<Tuple1>(t1)), std::get<I>(std::forward<Tuple2>(t2))))...);
  }

  template <class F, class Tuple1, class Tuple2>
  constexpr decltype(auto) apply2(F&& f, Tuple1&& t1, Tuple2&& t2) {
    return apply2_impl(std::forward<F>(f), std::forward<Tuple1>(t1), std::forward<Tuple2>(t2),
                      std::make_index_sequence<std::tuple_size<std::remove_reference_t<Tuple1>>::value>{});
  }

public:
  typedef Fn left_type;

  unzip_into_async(Fn first, Fns... fns) : fns_(first, fns...) {}

  template <typename... Args>
  decltype(auto) operator()(Args&&... args) {
    // We have a tuple of functions to run in parallel           - fns_
    // We have a parameter pack of args to UNZIP 
    // and then pass to each function - args...
    const auto bind_arg = [](auto&& fn, auto&& arg) {
      return pipeline::fn(std::bind(fn, std::move(arg)));
    };

    auto unzipped_fork_async = apply2(bind_arg, fns_, std::tuple<Args...>(args...));

    // Let's say the input args were (arg1, arg2, arg3, ...)
    // And we have functions (fn1, fn2, fn3, ...)

    // The above code has constructed a fork:
    // fork_async(bind(fn1, arg1), bind(fn2, arg2), bind(fn3, arg3), ...)
    //
    // Each function is bound to an argument from the input
    // and because it is a fork_async, these can/will run in parallel

    // Simply run the fork_async and return the results
    return unzipped_fork_async();
  }

  template <typename T3>
  auto operator|(T3&& rhs) {
    return pipe_pair<unzip_into<Fns...>, T3>(*this, std::forward<T3>(rhs));
  }

};

}