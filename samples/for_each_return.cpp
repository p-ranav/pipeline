#include <iostream>
#include <pipeline/pipeline.hpp>
using namespace pipeline;

int main() {

  auto generate_input = fn([] { return std::vector<int>{1, 2, 3, 4, 5}; });

  auto to_string = fn([](const auto &v) { return std::to_string(v); });

  auto print_string = fn([](const auto &vec_of_strings) {
    std::cout << "{ ";
    for (auto &s : vec_of_strings) {
      std::cout << s << " ";
    }
    std::cout << "}\n";
  });

  auto pipeline =
      generate_input | for_each(to_string) | print_string; // for_each returns a vector of results
  pipeline();
}