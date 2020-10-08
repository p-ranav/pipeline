#pragma once
#include <pipeline/details.hpp>
#include <pipeline/fn.hpp>
#include <functional>

namespace pipeline {

template <typename T>
auto from(T&& value) {
  return fn(std::bind([](auto&& value) -> auto&& { 
    return std::forward<decltype(value)>(value); 
  }, std::forward<T>(value)));
}

template <typename T>
auto from(T& value) {
  return fn([&value] () -> T& { return value; });
}

template <typename T>
auto from(const T& value) {
  return fn([&value] () -> const T& { return value; });
}

}