#include <pipeline/pipeline.hpp>
#include <iostream>
#include <algorithm>
using namespace pipeline;

int main() {
  std::vector<int> numbers{1, 2, 3, 4, 5};

  auto double_it = fn([](std::vector<int>& numbers) -> auto& {
    std::transform(numbers.begin(), numbers.end(), numbers.begin(), [](int a) { return a * 2; });
    return numbers;
  });

  auto square_it = fn([](std::vector<int>& numbers) -> auto& {
    std::transform(numbers.begin(), numbers.end(), numbers.begin(), [](int a) { return a * a; });
    return numbers;
  });

  auto pipeline = double_it | square_it;
  pipeline(numbers);

  for (auto&n : numbers) {
    std::cout << n << " ";
  }
  std::cout << std::endl;
}