#include <pipeline/fn.hpp>
#include <fstream>
#include <optional>
#include <sstream>
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
      | fork(add_3, square, cube)
      | fn([](auto offset, auto square, auto cube) { 
          std::cout << offset << " " << square << " " << cube << "\n"; 
        })
      ;
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

  {
    fn counter = [](auto n) { 
      static size_t previous{0};
      auto result = std::make_tuple(n, previous);
      previous = n;
      return result;
    };

    fn print_n = [](auto n, auto prev) {
      std::cout << "N = " << n << "\n";
      return n;
    };

    fn print_prev = [](auto n, auto prev) {
      std::cout << "Prev = " << prev << "\n";
      return prev;
    };

    fn print_result = [](auto n, auto prev) { 
      std::cout << "Stateful result = " << n << " - " << prev << "\n"; 
    };

    fn print_result_2 = [](auto packed) {
      std::cout << "Stateful result = " << std::get<0>(packed) << " - " << std::get<1>(packed) << "\n"; 
    };

    auto pipeline = counter | /* fork */ (print_n & print_prev) | /* join */ print_result;
    pipeline(5);
    pipeline(10);
    pipeline(15);
  }


  {
    fn read_file = [](std::string_view filename = "main.cpp") {
      std::string buffer;

      std::ifstream file(filename);
      file.seekg(0, std::ios::end);
      buffer.resize(file.tellg());
      file.seekg(0);
      file.read(buffer.data(), buffer.size());

      return buffer;
    };

    fn read_next_line = [](auto contents) -> std::optional<std::string> {
      static std::istringstream f(contents);
      std::string line;    
      if (std::getline(f, line)) {
        return line;
      }
      return {};
    };

    fn print_line = [](auto line) { if (line.has_value()) std::cout << line.value() << "\n"; };

    auto line_printer = read_file | read_next_line | print_line;

    line_printer();
    line_printer();
    line_printer();
  }
}