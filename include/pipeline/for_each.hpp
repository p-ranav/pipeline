#pragma once
#include <pipeline/details.hpp>
#include <pipeline/fn.hpp>
#include <vector>
#include <future>

namespace pipeline {

template <typename Fn> class for_each {
  Fn fn_;

public:
  for_each(Fn fn) : fn_(fn) {}

  template <typename Container> decltype(auto) operator()(Container &&args) {
    typedef typename std::result_of<Fn(typename Container::value_type &)>::type result_type;

    if constexpr (std::is_same<result_type, void>::value) {
      // result type is void
      std::vector<std::future<result_type>> futures;
      for (auto &arg : std::forward<Container>(args)) {
        futures.push_back(std::async(std::launch::async | std::launch::deferred, fn_, arg));
      }

      for (auto &f : futures) {
        f.get();
      }
    } else {
      // result is not void
      std::vector<result_type> results;
      std::vector<std::future<result_type>> futures;
      for (auto &arg : std::forward<Container>(args)) {
        futures.push_back(std::async(std::launch::async | std::launch::deferred, fn_, arg));
      }

      for (auto &f : futures) {
        results.push_back(f.get());
      }
      return results;
    }
  }
};

} // namespace pipeline