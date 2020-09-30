#pragma once
#include <pipeline/details.hpp>

namespace pipeline {

template <typename T1, typename T2>
class pipe_pair;

template <typename T1, typename T2>
class fork_pair;

template <typename Fn, typename... Args>
class bind {
  Fn fn_;
  std::tuple<Args...> args_;

public:
  typedef Fn left_type;

  bind(Fn fn, Args... args): fn_(fn), args_(args...) {}

  template <typename... T>
  auto operator()(T&&... left_args) {
    return details::apply(std::tuple_cat(std::forward_as_tuple(std::forward<T>(left_args)...), args_), fn_);
  }

  template <typename... A>
  static constexpr bool is_invocable_on() {
    return std::is_invocable<Fn, A...>::value;
  }

  // If rhs is fork, bind or pipe
  template <typename T>
  typename std::enable_if<
    details::is_specialization<typename std::decay<T>::type, bind>::value || 
    details::is_specialization<typename std::decay<T>::type, pipe_pair>::value || 
    details::is_specialization<typename std::decay<T>::type, fork_pair>::value, 
  pipe_pair<bind<Fn, Args...>, T>>::type 
  operator|(T&& rhs) {
    return pipe_pair<bind<Fn, Args...>, T>(*this, std::forward<T>(rhs));
  }

  // If rhs is a lambda function
  template <typename T>
  typename std::enable_if<
    !details::is_specialization<typename std::decay<T>::type, bind>::value &&
    !details::is_specialization<typename std::decay<T>::type, pipe_pair>::value &&
    !details::is_specialization<typename std::decay<T>::type, fork_pair>::value, 
  pipe_pair<bind<Fn, Args...>, bind<T>>>::type 
  operator|(T&& rhs) {
    return pipe_pair<bind<Fn, Args...>, bind<T>>(*this, bind<T>(std::forward<T>(rhs)));
  }
};

}