#include <pipeline/pipeline.hpp>
#include <iostream>
using namespace pipeline;

int main() {

  auto generate_input = fn([] {
    return std::vector<int>{1, 2, 3, 4, 5};
  });

  auto double_it = fn([](const auto& v) {
    return v * 2;
  });

  auto print_vec = fn([](std::vector<std::future<int>>& futures) {
    for (auto& f: futures) {
      std::cout << f.get() << std::endl;
    }
  });

  auto pipeline = generate_input | for_each_async(double_it) | print_vec;
  pipeline();

}