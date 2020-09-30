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

template <typename T1, typename T2>
class fork_pair {
  T1 left_;
  T2 right_;

  template <typename... T>
  auto apply_left(std::tuple<T...> args) {
    // check if left_ can take a tuple
    if constexpr (is_invocable_on<T1, std::tuple<T...>>()) {
      // left_ takes a tuple
      return left_(args);
    } else {
      // unpack tuple into parameter pack and call left_
      return details::apply(args, left_);
    }
  }

  template <typename... T>
  auto apply_left(T&&... args) {
    return left_(std::forward<T>(args)...);
  }

  template <typename... T>
  auto apply_right(std::tuple<T...> args) {
    // check if right_ can take a tuple
    if constexpr (is_invocable_on<T2, std::tuple<T...>>()) {
      // right_ takes a tuple
      return right_(args);
    } else {
      // unpack tuple into parameter pack and call right_
      return details::apply(args, right_);
    }
  }

  template <typename... T>
  auto apply_right(T&&... args) {
    return right_(std::forward<T>(args)...);
  }

public:
  typedef T1 left_type;
  typedef T2 right_type;

  fork_pair(T1 left, T2 right) : left_(left), right_(right) {}

  // output of fork_pair(...) is always a tuple
  // - a tuple of merged results of left and right
  template <typename... T>
  auto operator()(T&&... args) {
    typedef typename std::result_of<T1(T...)>::type left_result;
    typedef typename std::result_of<T2(T...)>::type right_result;

    // check if the results are tuple
    // The result of this function is a tuple
    // If left_ and right_ return tuples, merge the results with std::tuple_cat and return
    if constexpr (details::is_tuple<left_result>::value) {
      if constexpr (details::is_tuple<right_result>::value) {
        // both results are tuples
        // unpack both results and pack as tuple the concat of each result
        return std::tuple_cat(apply_left(std::forward<T>(args)...), apply_right(std::forward<T>(args)...));
      } else {
        // left result alone is a tuple
        left_result l = apply_left(std::forward<T>(args)...);

        // right result could be void
        if constexpr (std::is_same<right_result, void>::value) {
          apply_right(std::forward<T>(args)...);
          return std::make_tuple(l);
        } else {
          return std::tuple_cat(l, std::make_tuple(apply_right(std::forward<T>(args)...)));
        }
      }
    }
    else if constexpr (details::is_tuple<right_result>::value) {
      // right result is a tuple
      right_result r = apply_right(std::forward<T>(args)...);

      // left result could be void
      if constexpr (std::is_same<left_result, void>::value) {
        apply_left(std::forward<T>(args)...);
        return std::make_tuple(r);
      } else {
        return std::tuple_cat(std::make_tuple(apply_left(std::forward<T>(args)...)), r);
      }
    }
    else {
      // neither result is a tuple

      // if left_result is a void
      if constexpr (std::is_same<left_result, void>::value && !std::is_same<right_result, void>::value) {
        apply_left(std::forward<T>(args)...);
        return std::make_tuple(apply_right(std::forward<T>(args)...));
      }
      // if right_result is a void
      else if constexpr (!std::is_same<left_result, void>::value && std::is_same<left_result, void>::value) {
        auto l = apply_left(std::forward<T>(args)...);
        apply_right(std::forward<T>(args)...);
        return std::make_tuple(l);
      }
      // if both results are void
      else if constexpr (std::is_same<left_result, void>::value && std::is_same<left_result, void>::value) {
        apply_left(std::forward<T>(args)...);
        apply_right(std::forward<T>(args)...);
        return std::make_tuple();
      }
      // neither result is void
      else {
        return std::make_tuple(apply_left(std::forward<T>(args)...), apply_right(std::forward<T>(args)...));
      }
    }
  }

  template <typename F, typename... Args>
  static constexpr bool is_invocable_on() {
    if constexpr (details::is_specialization<typename std::remove_reference<F>::type, bind>::value) {
      // F is an `bind` type
      return std::remove_reference<F>::type::template is_invocable_on<Args...>();
    }
    else {
      return is_invocable_on<typename F::left_type, Args...>();
    }
  }

  // If rhs is fork, bind or pipe
  template <typename T3>
  typename std::enable_if<
    details::is_specialization<typename std::decay<T3>::type, bind>::value || 
    details::is_specialization<typename std::decay<T3>::type, pipe_pair>::value || 
    details::is_specialization<typename std::decay<T3>::type, fork_pair>::value, 
  pipe_pair<fork_pair<T1, T2>, T3>>::type 
  operator|(T3&& rhs) {
    return pipe_pair<fork_pair<T1, T2>, T3>(*this, std::forward<T3>(rhs));
  }

  // If rhs is a lambda function
  template <typename T3>
  typename std::enable_if<
    !details::is_specialization<typename std::decay<T3>::type, bind>::value &&
    !details::is_specialization<typename std::decay<T3>::type, pipe_pair>::value &&
    !details::is_specialization<typename std::decay<T3>::type, fork_pair>::value, 
  pipe_pair<fork_pair<T1, T2>, bind<T3>>>::type 
  operator|(T3&& rhs) {
    return pipe_pair<fork_pair<T1, T2>, bind<T3>>(*this, bind(std::forward<T3>(rhs)));
  }
};

template <typename T1, typename T2>
auto fork(T1&& t1, T2&& t2) {
  return fork_pair<T1, T2>(std::forward<T1>(t1), std::forward<T2>(t2));
}

template <typename T1, typename T2, typename... T>
auto fork(T1&& t1, T2&& t2, T&&... args) {
  return fork(fork<T1, T2>(std::forward<T1>(t1), std::forward<T2>(t2)), std::forward<T>(args)...);
}

template <typename... T>
auto fork_parallel(T&&... args) {
  return bind([](T... args) {
    return std::make_tuple(std::async(std::launch::async, args)...);
  }, std::forward<T>(args)...);
}

}