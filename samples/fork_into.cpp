#include <iostream>
#include <pipeline/pipeline.hpp>
using namespace pipeline;

int main() {

  auto double_it = [](auto vec) {
    std::cout << "Doubling\n";
    std::transform(vec.begin(), vec.end(), vec.begin(), [](auto &e) { return e * 2; });
    return vec;
  };

  auto square_it = [](auto vec) {
    std::cout << "Squaring\n";
    std::transform(vec.begin(), vec.end(), vec.begin(), [](auto &e) { return e * e; });
    return vec;
  };

  auto print_vecs = [](auto results) {
    auto doubled = results[0];
    auto squared = results[1];

    for (auto &e : doubled) {
      std::cout << e << " ";
    }
    std::cout << "\n";

    for (auto &e : squared) {
      std::cout << e << " ";
    }
    std::cout << "\n";
  };

  auto pipeline = from(std::vector<int>{1, 2, 3, 4, 5}) | fork_into(double_it, square_it) | print_vecs;
  pipeline();
}