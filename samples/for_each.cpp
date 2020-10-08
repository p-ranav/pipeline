#include <iostream>
#include <pipeline/pipeline.hpp>
using namespace pipeline;

int main() {

  auto generate_input = fn([] { return std::vector<int>{1, 2, 3, 4, 5}; });

  auto print_values = fn([](const auto &v) { std::cout << v << "\n"; });

  auto pipeline = generate_input | for_each(print_values);
  pipeline();
}