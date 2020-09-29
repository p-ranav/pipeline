#pragma once
#include <pipeline/details.hpp>
#include <pipeline/pipe.hpp>

namespace pipeline {

template <typename Fn, typename... Args>
class fn;

template <typename T1, typename T2>
class fork {
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

  fork(T1 left, T2 right) : left_(left), right_(right) {}

  // output of fork is always a tuple
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
    if constexpr (details::is_specialization<F, fn>::value) {
      // F is an `fn` type
      return F::template is_invocable_on<Args...>();
    }
    else {
      return is_invocable_on<typename F::left_type, Args...>();
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

}