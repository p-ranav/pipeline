#include <pipeline/pipeline.hpp>
#include <iostream>
using namespace pipeline;

int main() {

  auto generate_input = fn([] {
    return std::vector<int>{1, 2, 3, 4, 5};
  });

  auto print_value = fn([](const auto& v) {
    std::cout << v << "\n";
    return std::to_string(v);
  });

  auto print_string = fn([](const auto& vec_of_strings) {
    std::cout << "{ ";
    for (auto& s: vec_of_strings) {
      std::cout << s << " ";
    }
    std::cout << "}\n";
  });

  auto pipeline = generate_input | for_each(print_value) | print_string;
  pipeline();

}