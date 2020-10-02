#pragma once
#include <pipeline/details.hpp>

namespace pipeline {

template <typename T1, typename T2>
class pipe_pair;

template <typename Fn>
class fn {
  Fn fn_;

public:
  typedef Fn left_type;

  fn(Fn fn): fn_(fn) {}

  template <typename... T>
  auto operator()(T&&... args) {
    return details::apply(std::forward_as_tuple(std::forward<T>(args)...), fn_);
  }

  template <typename... A>
  static constexpr bool is_invocable_on() {
    return std::is_invocable<Fn, A...>::value;
  }

  template <typename T>
  auto operator|(T&& rhs) {
    return pipe_pair<fn<Fn>, T>(*this, std::forward<T>(rhs));
  }
};

}