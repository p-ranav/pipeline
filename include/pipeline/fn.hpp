#pragma once
#include <pipeline/pipe.hpp>
#include <pipeline/fork.hpp>

namespace pipeline {

template <typename Fn, typename... Args>
class fn {
  Fn fn_;
  std::tuple<Args...> args_;

public:
  typedef Fn left_type;

  fn(Fn fn, Args... args): fn_(fn), args_(args...) {}

  template <typename... T>
  auto operator()(T&&... left_args) {
    return details::apply(std::tuple_cat(std::forward_as_tuple(std::forward<T>(left_args)...), args_), fn_);
  }

  template <typename... A>
  static constexpr bool is_invocable_on() {
    return std::is_invocable<Fn, A...>::value;
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