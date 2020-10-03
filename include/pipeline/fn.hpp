#pragma once
#include <pipeline/details.hpp>

namespace pipeline {

template <typename Fn>
class fn {
  Fn fn_;

public:
  fn(Fn fn): fn_(fn) {}

  template <typename... T>
  decltype(auto) operator()(T&&... args) {
    return fn_(args...);
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