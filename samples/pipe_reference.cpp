#include <algorithm>
#include <iostream>
#include <pipeline/pipeline.hpp>
using namespace pipeline;

int main() {
  std::vector<int> numbers{1, 2, 3, 4, 5};

  auto double_it = [](std::vector<int> & numbers) -> auto & {
    std::transform(numbers.begin(), numbers.end(), numbers.begin(), [](int a) { return a * 2; });
    return numbers;
  };

  auto square_it = [](std::vector<int> & numbers) -> auto & {
    std::transform(numbers.begin(), numbers.end(), numbers.begin(), [](int a) { return a * a; });
    return numbers;
  };

  auto pipeline = from(numbers) | double_it | square_it;
  pipeline();

  for (auto &n : numbers) {
    std::cout << n << " ";
  }
  std::cout << std::endl;
  // 4 16 36 64 100
}