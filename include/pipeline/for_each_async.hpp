#pragma once
#include <pipeline/details.hpp>
#include <pipeline/fn.hpp>
#include <vector>

namespace pipeline {

template <typename Fn>
class for_each_async {
  Fn fn_;

public:
  for_each_async(Fn fn) : fn_(fn) {}

  template <typename Container>
  decltype(auto) operator()(Container&& args) {
    typedef typename std::result_of<Fn(typename Container::value_type&)>::type result_type;
    std::vector<std::future<result_type>> futures;
    for (auto& arg: std::forward<Container>(args)) {
      futures.push_back(std::async(std::launch::async | std::launch::deferred, fn_, arg));
    }
    return std::move(futures);
  }
};  

}