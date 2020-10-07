#include <pipeline/pipeline.hpp>
#include <iostream>
#include <string>
using namespace pipeline;

int main() {
  auto generate_input = fn([] {
    return std::tuple<int, float, std::string>{1, 3.14f, "Hello World"};
  });

  auto printer = fn([](auto a) { std::cout << a << "\n"; });

  auto pipeline = generate_input | unzip_into(printer);
  pipeline();

}