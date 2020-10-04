#include <pipeline/pipeline.hpp>
using namespace pipeline;
#include <iostream>

int main() {
  auto generate_input = [] { return std::make_tuple("hello ", "world!"); };

  auto title_case = [] (std::string input) { 
    input[0] = toupper(input[0]);
    return input;
  };

  auto concat = [] (auto&& input_1, auto&& input_2) { return std::move(input_1 + input_2); };

  auto print_output = [](auto&& input) { std::cout << input << "\n"; };

  auto pipeline = fn(generate_input) | unzip_into(title_case) | concat | print_output;
  pipeline();
}