#include <pipeline/pipeline.hpp>
#include <iostream>
#include <string>
using namespace pipeline;

int main() {
  auto generate_input = fn([] {
    return std::make_tuple(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
  });

  auto is_even = fn([](auto a) { return a % 2 == 0; });

  auto print_results = fn([](auto&& results) {
    for (const auto& r: results) {
      std::cout << std::boolalpha << r << "\n";
    }
  });

  auto pipeline = generate_input | unzip_into(is_even) | print_results;
  pipeline();

}