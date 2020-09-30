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

  template <typename T>
  auto operator|(T&& rhs) {
    return pipe_pair(*this, std::forward<T>(rhs));
  }
};

}