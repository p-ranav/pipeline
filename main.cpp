#include <pipeline/fn.hpp>
using namespace pipeline;

int main() {
 
  {
    auto add = fn([](int a, int b) { return a + b; });
    auto square = fn([](int a) { return a * a; });
    auto pretty_print_square = fn([](int result, std::string msg = "Result = ") { std::cout << msg << std::to_string(result); });

    auto pipeline = add | square | pretty_print_square;
    pipeline(5, 10);
  }

  std::cout << std::endl;


  {
    auto input = fn([]() { return std::vector<int>{1, 2, 3, 4, 5}; });
    auto square = fn([](const std::vector<int>& input) {
      std::vector<int> result;
      for (auto i : input) result.push_back(i * i);
      return result;
    });
    auto reverse = fn([](const std::vector<int>& input) {
      std::vector<int> result;
      for (auto i = input.rbegin(); i != input.rend(); i++) {
        result.push_back(*i);
      }
      return result;
    });

    auto to_string = fn([](const std::vector<int>& input) {
      std::string result = "{ ";
      for (auto i : input) {
        result += std::to_string(i) + " ";
      }
      result += "}";
      return result;
    });

    auto pipeline = square | reverse | to_string;

    // apply for a single input
    std::cout << pipeline(std::vector<int>{1, 2, 3, 4, 5}) << "\n";

    // apply to a vector of inputs
    std::vector<std::vector<int>> inputs = {
      {1, 2, 3, 4, 5},
      {6, 7, 8, 9, 10},
      {11, 12, 13, 14, 15}
    };
    std::vector<std::string> result{3};
    std::transform(inputs.begin(), inputs.end(), result.begin(), pipeline);
    for (auto r : result) {
      std::cout << r << "\n";
    }
  }

  {
    auto square = fn([](auto n) { 
      std::transform(n.begin(), n.end(), n.begin(), [](auto i) { return i * i; });
      return n;
    });

    auto mean = fn([](auto n) {
      return std::accumulate(n.begin(), n.end(), 0) / n.size();
    });

    auto root = fn([](auto n) {
      return std::sqrt(n);
    });

    auto rms = square | mean | root;
    std::cout << rms(std::vector<int>{2, 4, 9, 10, 12}) << "\n"; // 8.30662
  }

  // Unpack fork result (which is a tuple) 
  // and call next fn
  {
    auto square = fn([](int a) { return a * a; });
    auto cube = fn([](int a) { return a * a * a; });
    auto pipeline = 
      square
      | (square & cube)
      | fn([](auto square_result, auto cube_result) { 
          std::cout << square_result << " " << cube_result << "\n"; 
        });
    pipeline(5);
  }

  // Unpack fork result (which is a tuple) 
  // and call next fn
  {
    auto add_3 = fn([](int a) { return a + 3; });
    auto square = fn([](int a) { return a * a; });
    auto cube = fn([](int a) { return a * a * a; });
    auto pipeline = 
      square
      | (add_3 & square & cube)
      | fn([](auto offset, auto square, auto cube) { 
          std::cout << offset << " " << square << " " << cube << "\n"; 
        })
      ;
    pipeline(5);
  }

  // Keep fork result as a tuple
  // and call next fn
  {
    auto add_10 = fn([](int a) { return a + 10; });
    auto square = fn([](int a) { return a * a; });
    auto cube = fn([](int a) { return a * a * a; });
    auto pipeline = 
      square
      | ((add_10 | square) & square & cube)
      | fn([](auto packed_result) { 
          std::cout << std::get<0>(packed_result) << " " << std::get<1>(packed_result) << " " << std::get<2>(packed_result)<< "\n"; 
        })
      ;
    pipeline(5);
  }

  // Piping a tuple
  {
    auto make_tuple = fn([]() { return std::make_tuple(5, 3.14f, std::string{"Hello World"}); });

    auto print = fn([](auto int_a, auto float_b, auto str_c) {
      std::cout << int_a << " " << float_b << " " << str_c << "\n";
    });

    auto pipeline = make_tuple | print;
    pipeline();
  }
}