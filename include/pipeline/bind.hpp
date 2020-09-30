#pragma once
#include <pipeline/fork.hpp>
#include <pipeline/pipe.hpp>

namespace pipeline {

template <typename Fn, typename... Args>
class bind {
  Fn bind_;
  std::tuple<Args...> args_;

public:
  typedef Fn left_type;

  bind(Fn bind, Args... args): bind_(bind), args_(args...) {}

  template <typename... T>
  auto operator()(T&&... left_args) {
    return details::apply(std::tuple_cat(std::forward_as_tuple(std::forward<T>(left_args)...), args_), bind_);
  }

  template <typename... A>
  static constexpr bool is_invocable_on() {
    return std::is_invocable<Fn, A...>::value;
  }
  
  template <typename Fn2, typename... Args2>
  auto operator|(const bind<Fn2, Args2...>& rhs) {
    return pipe_pair(*this, rhs);
  }

  template <typename T1, typename T2>
  auto operator|(const pipe_pair<T1, T2>& rhs) {
    return pipe_pair(*this, rhs);
  }

  template <typename T1, typename T2>
  auto operator|(const fork_pair<T1, T2>& rhs) {
    return pipe_pair(*this, rhs);
  }

  template <typename Fn2, typename... Args2>
  auto operator&(const bind<Fn2, Args2...>& rhs) {
    return fork_pair(*this, rhs);
  }
};

}