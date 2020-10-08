#include <iostream>
#include <pipeline/pipeline.hpp>
using namespace pipeline;

int main() {
  auto print_values = [](const auto &v) { std::cout << v << "\n"; };

  auto pipeline = from(std::vector<int>{1, 2, 3, 4, 5}) | for_each(print_values);
  pipeline();
}