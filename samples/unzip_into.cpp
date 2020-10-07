#include <pipeline/pipeline.hpp>
#include <iostream>
#include <string>
using namespace pipeline;

int main() {

  auto generate_input = fn([] {
    return std::tuple<int, float, std::string>{1, 3.14f, "Hello World"};
  });

  auto handle_int = fn([](int a) {
    std::cout << a << "\n";
  });

  auto handle_float = fn([](float a) {
    std::cout << a << "f\n";
  });

  auto handle_string = fn([](std::string a) {
    std::cout << "\"" << a << "\"\n";
  });

  auto pipeline = generate_input | unzip_into(handle_int, handle_float, handle_string);
  pipeline();

}