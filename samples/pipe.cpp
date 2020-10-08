#include <pipeline/pipeline.hpp>
using namespace pipeline;
#include <iostream>

int main() {
  auto add = fn([](auto a, auto b) { return a + b; });
  auto double_it = fn([](auto a) { return a * 2; });
  auto square_it = fn([](auto a) { return a * a; });
  auto pipeline = add | double_it | square_it;

  std::cout << pipeline(3, 6) << "\n";
  std::cout << pipeline(4.5, 9.3) << "\n";
}