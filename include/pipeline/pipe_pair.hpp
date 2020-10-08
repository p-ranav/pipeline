#pragma once
#include <pipeline/details.hpp>

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