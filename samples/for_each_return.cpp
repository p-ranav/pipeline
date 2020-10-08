#include <iostream>
#include <pipeline/pipeline.hpp>
using namespace pipeline;

int main() {

  auto to_string = [](const auto &v) { return std::to_string(v); };

  auto print_string = [](const auto &vec_of_strings) {
    std::cout << "{ ";
    for (auto &s : vec_of_strings) {
      std::cout << s << " ";
    }
    std::cout << "}\n";
  };

  auto pipeline =
      from(std::vector<int>{1, 2, 3, 4, 5}) | for_each(to_string) | print_string; // for_each returns a vector of results
  pipeline();
}