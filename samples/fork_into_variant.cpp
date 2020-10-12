#include <iostream>
#include <pipeline/pipeline.hpp>
using namespace pipeline;
#include <vector>
#include <variant>

int main() {

  auto generate_input = fn([] { return std::vector<int>{1, 2, 3, 4, 5}; });

  auto forward_it = [](auto vec) -> std::variant<std::vector<int>, std::string> {
    return vec;
  };

  auto string_it = [](auto vec) -> std::variant<std::vector<int>, std::string> {
    std::string result = "{ ";
    for (auto& v: vec) { result += std::to_string(v) + " "; }
    result += "}";
    return result;
  };

  auto print_vecs = [](auto results) {

    for (auto& r: results) {
      if (std::holds_alternative<std::string>(r)) {
        // result is a string, print it
        std::cout << "Stringified : " << std::get<std::string>(r) << "\n";
      } else {
        // result is a vector
        std::cout << "Forwarded   : ";
        for (auto &e : std::get<std::vector<int>>(r)) {
          std::cout << e << " ";
        }    
        std::cout << "\n";
      }
    }
  };

  auto pipeline = generate_input | fork_into(forward_it, string_it) | print_vecs;
  pipeline();
}