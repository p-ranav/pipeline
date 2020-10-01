#pragma once
#include <pipeline/details.hpp>

namespace pipeline {

template <typename Fn, typename... Args>
class bind;

template <typename Fn, typename... Fns>
class fork;

template <typename T1, typename T2>
class pipe_pair {
  T1 left_;
  T2 right_;

public:
  typedef T1 left_type;
  typedef T2 right_type;

  pipe_pair(T1 left, T2 right) : left_(left), right_(right) {}

  template <typename... T>
  auto operator()(T&&... args) {
    typedef typename std::result_of<T1(T...)>::type left_result;

    // check if left_ result is a tuple
    if constexpr (details::is_tuple<left_result>::value) {
      // left_ result is a tuple

      if constexpr (is_invocable_on<T2, left_result>()) {
        // right_ takes a tuple
        return right_(left_(std::forward<T>(args)...));
      } else {

        // check if right is invocable without args
        if constexpr (is_invocable_on<T2>()) {
          return right_();
        } else {
          // unpack tuple into parameter pack and call right_
          return details::apply(left_(std::forward<T>(args)...), right_);
        }
      }
    } else {
      // left_result not a tuple
      // call right_ with left_result
      if constexpr (!std::is_same<left_result, void>::value) {
        return right_(left_(std::forward<T>(args)...));
      } else {
        // left result is void
        left_(std::forward<T>(args)...);
        return right_();
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
    details::is_specialization<typename std::decay<T3>::type, fork>::value, 
  pipe_pair<pipe_pair<T1, T2>, T3>>::type 
  operator|(T3&& rhs) {
    return pipe_pair<pipe_pair<T1, T2>, T3>(*this, std::forward<T3>(rhs));
  }

  // If rhs is a lambda function
  template <typename T3>
  typename std::enable_if<
    !details::is_specialization<typename std::decay<T3>::type, bind>::value &&
    !details::is_specialization<typename std::decay<T3>::type, pipe_pair>::value &&
    !details::is_specialization<typename std::decay<T3>::type, fork>::value, 
  pipe_pair<pipe_pair<T1, T2>, bind<T3>>>::type 
  operator|(T3&& rhs) {
    return pipe_pair<pipe_pair<T1, T2>, bind<T3>>(*this, bind(std::forward<T3>(rhs)));
  }
};

template <typename T1, typename T2>
auto pipe(T1&& t1, T2&& t2) {
  return pipe_pair<T1, T2>(std::forward<T1>(t1), std::forward<T2>(t2));
}

template <typename T1, typename T2, typename... T>
auto pipe(T1&& t1, T2&& t2, T&&... args) {
  return pipe(pipe<T1, T2>(std::forward<T1>(t1), std::forward<T2>(t2)), std::forward<T>(args)...);
}

}