#include <pipeline/pipeline.hpp>
using namespace pipeline;
#include <iostream>

int main() {
  auto generate_input = fn([] { return std::make_tuple(12, 3); });

  auto double_it = fn([](auto a, auto b) { return std::make_tuple(a * 2, b * 2); });

  auto sum = fn([](auto a, auto b) { return a + b; });
  auto diff = fn([](auto a, auto b) { return a - b; });

  auto print_results = fn([](auto&& sum, auto&& diff) { // sum and diff are std::future<int>
    std::cout << "Sum = " << sum.get() << ", Diff = " << diff.get() << std::endl;
  });

  auto pipeline = generate_input | double_it | fork_async(sum, diff) | print_results;
  pipeline();
}

// prints:
// Sum = 30, Diff = 18