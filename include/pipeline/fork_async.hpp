#pragma once
#include <pipeline/details.hpp>
#include <thread>
#include <future>
#include <pipeline/bind.hpp>

namespace pipeline {

template <typename Fn, typename... Args>
class bind;

template <typename T1, typename T2>
class pipe_pair;

// Very similar to fork
// Unlike fork, fork_async::operator() does not wait for results
// - it simply returns a tuple of futures
template <typename Fn, typename... Fns>
class fork_async {
  Fn first_;
  std::tuple<Fn, Fns...> fns_;

  // Takes a tuple of arguments          : args_tuple
  // Takes a parameter pack of functions : fns
  //
  // Converts the args_tuple into a parameter pack
  // Calls std::async and spawns thread, one for each function
  // Each thread will run the function with the parameter pack of arguments
  //
  // Waits for all the threads to finish - we will get a tuple of futures
  // Returns a tuple of future.get() values
  // For functions that return `void`, we simply return true to indicate that
  // the function completed
  template <typename A, typename... T>
  auto do_fork_async(A&& args_tuple, T&&... fns) {
    return bind([](A args_tuple, T... fns) {

      auto unpack = [](auto tuple, auto fn) {
        return details::apply(tuple, fn);
      };

      return std::make_tuple(std::async(std::launch::async | std::launch::deferred, unpack, args_tuple, fns)...);
    }, std::forward<A>(args_tuple), std::forward<T>(fns)...);
  }

public:
  typedef Fn left_type;

  fork_async(Fn first, Fns... fns) : first_(first), fns_(first, fns...) {}

  template <typename... Args>
  auto operator()(Args&&... args) {
    // We have a tuple of functions to run in parallel           - fns_
    // We have a parameter pack of args to pass to each function - args...
    // 
    // The following applies a lambda function to the fns_ tuple
    // the fns_ tuple is converted into a parameter pack and 
    // the lambda is called with that parameter pack
    // 
    // Then, we call do_fork_async which takes a tuple of args
    // along with the parameter pack of functions
    return details::apply(fns_, [this, args...](auto... fns) {
      return do_fork_async(std::make_tuple(args...), fns...)();
    });
  }

  // If rhs is bind or pipe, fork or fork_async
  template <typename T3>
  typename std::enable_if<
    details::is_specialization<typename std::decay<T3>::type, bind>::value || 
    details::is_specialization<typename std::decay<T3>::type, pipe_pair>::value || 
    details::is_specialization<typename std::decay<T3>::type, fork>::value ||
    details::is_specialization<typename std::decay<T3>::type, fork_async>::value, 
  pipe_pair<fork_async<Fn, Fns...>, T3>>::type 
  operator|(T3&& rhs) {
    return pipe_pair<fork_async<Fn, Fns...>, T3>(*this, std::forward<T3>(rhs));
  }

  // If rhs is a lambda function
  template <typename T3>
  typename std::enable_if<
    !details::is_specialization<typename std::decay<T3>::type, bind>::value &&
    !details::is_specialization<typename std::decay<T3>::type, pipe_pair>::value &&
    !details::is_specialization<typename std::decay<T3>::type, fork>::value &&
    !details::is_specialization<typename std::decay<T3>::type, fork_async>::value, 
  pipe_pair<fork_async<Fn, Fns...>, bind<T3>>>::type 
  operator|(T3&& rhs) {
    return pipe_pair<fork_async<Fn, Fns...>, bind<T3>>(*this, bind(std::forward<T3>(rhs)));
  }

};

}