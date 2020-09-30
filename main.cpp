#include <pipeline/bind.hpp>
#include <pipeline/pipe.hpp>
#include <pipeline/fork.hpp>
#include <fstream>
#include <optional>
#include <sstream>
#include <functional>
#include "string_utils.hpp"
using namespace pipeline;

int main() {
 
  {
    auto add = bind([](int a, int b) { return a + b; }, 5, 10);
    auto square = bind([](int a) { return a * a; });
    auto pretty_print_square = bind([](int result, std::string msg = "Result = ") { std::cout << msg << std::to_string(result) << "\n"; });

    auto pipeline = add | square | pretty_print_square;
    pipeline();
  }

  // Using `pipe(...)` function
  {
    auto add = bind([](int a, int b) { return a + b; });
    auto square = bind([](int a) { return a * a; });
    auto pretty_print_square = bind([](int result, std::string msg = "Result = ") { std::cout << msg << std::to_string(result) << "\n"; });

    auto pipeline = pipe(add, square, pretty_print_square);
    pipeline(15, 23);
  }

  {
    auto input = bind([]() { return std::vector<int>{1, 2, 3, 4, 5}; });
    auto square = bind([](const std::vector<int>& input) {
      std::vector<int> result;
      for (auto i : input) result.push_back(i * i);
      return result;
    });
    auto reverse = bind([](const std::vector<int>& input) {
      std::vector<int> result;
      for (auto i = input.rbegin(); i != input.rend(); i++) {
        result.push_back(*i);
      }
      return result;
    });

    auto to_string = bind([](const std::vector<int>& input) {
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
    auto square = bind([](auto n) { 
      std::transform(n.begin(), n.end(), n.begin(), [](auto i) { return i * i; });
      return n;
    });

    auto mean = bind([](auto n) {
      return std::accumulate(n.begin(), n.end(), 0) / n.size();
    });

    auto root = bind([](auto n) {
      return std::sqrt(n);
    });

    auto rms = square | mean | root;
    std::cout << rms(std::vector<int>{2, 4, 9, 10, 12}) << "\n"; // 8.30662
  }

  // Unpack fork result (which is a tuple) 
  // and call next bind
  {
    auto square = bind([](int a) { return a * a; });
    auto cube = bind([](int a) { return a * a * a; });
    auto pipeline = 
      square
      | fork(square, cube)
      | bind([](auto square_result, auto cube_result) { 
          std::cout << square_result << " " << cube_result << "\n"; 
        });
    pipeline(5);
  }

  // Unpack fork result (which is a tuple) 
  // and call next bind
  {
    auto add_3 = bind([](int a) { return a + 3; });
    auto square = bind([](int a) { return a * a; });
    auto cube = bind([](int a) { return a * a * a; });
    auto pipeline = 
      square
      | fork(add_3, square, cube)
      | bind([](auto offset, auto square, auto cube) { 
          std::cout << offset << " " << square << " " << cube << "\n"; 
        })
      ;
    pipeline(5);
  }

  // Unpack fork result (which is a tuple) 
  // and call next bind
  {
    auto add_3 = bind([](int a) { return a + 3; });
    auto square = bind([](int a) { return a * a; });
    auto cube = bind([](int a) { return a * a * a; });
    auto pipeline = 
      square
      | fork(add_3, square, cube)
      | bind([](auto offset, auto square, auto cube) { 
          std::cout << offset << " " << square << " " << cube << "\n"; 
        })
      ;
    pipeline(5);
  }

  // Keep fork result as a tuple
  // and call next bind
  {
    auto add_10 = bind([](int a) { return a + 10; });
    auto square = bind([](int a) { return a * a; });
    auto cube = bind([](int a) { return a * a * a; });
    auto pipeline = 
      square
      | fork((add_10 | square), square, cube)
      | bind([](auto packed_result) { 
          std::cout << std::get<0>(packed_result) << " " << std::get<1>(packed_result) << " " << std::get<2>(packed_result)<< "\n"; 
        })
      ;
    pipeline(5);
  }

  // Piping a tuple
  {
    auto make_tuple = bind([]() { return std::make_tuple(5, 3.14f, std::string{"Hello World"}); });

    auto print = bind([](auto int_a, auto float_b, auto str_c) {
      std::cout << int_a << " " << float_b << " " << str_c << "\n";
    });

    auto pipeline = make_tuple | print;
    pipeline();
  }

  {
    bind counter = [](auto n) { 
      static size_t previous{0};
      auto result = std::make_tuple(n, previous);
      previous = n;
      return result;
    };

    bind print_n = [](auto n, auto prev) {
      std::cout << "N = " << n << "\n";
      return n;
    };

    bind print_prev = [](auto n, auto prev) {
      std::cout << "Prev = " << prev << "\n";
      return prev;
    };

    bind print_result = [](auto n, auto prev) { 
      std::cout << "Stateful result = " << n << " - " << prev << "\n"; 
    };

    bind print_result_2 = [](auto packed) {
      std::cout << "Stateful result = " << std::get<0>(packed) << " - " << std::get<1>(packed) << "\n"; 
    };

    auto pipeline = counter | fork(print_n, print_prev) | print_result;
    pipeline(5);
    pipeline(10);
    pipeline(15);
  }

  {
    bind read_file = [](std::string_view filename = "main.cpp") {
      std::string buffer;

      std::ifstream file(filename);
      file.seekg(0, std::ios::end);
      buffer.resize(file.tellg());
      file.seekg(0);
      file.read(buffer.data(), buffer.size());

      return buffer;
    };

    bind read_next_line = [](auto contents) -> std::optional<std::string> {
      static std::istringstream f(contents);
      std::string line;    
      if (std::getline(f, line)) {
        return line;
      }
      return {};
    };

    bind print_line = [](auto line) { if (line.has_value()) std::cout << line.value() << "\n"; };

    auto line_printer = read_file | read_next_line | print_line;

    line_printer();
    line_printer();
    line_printer();
  }

  {
    bind t1 = []() { std::cout << "TaskA\n"; };
    bind t2 = []() { std::cout << "TaskB\n"; };
    bind t3 = []() { std::cout << "TaskC\n"; };
    bind t4 = []() { std::cout << "TaskD\n"; };
    auto pipeline = t1 | fork(t2, t3) | t4;
    pipeline();
  }

  {
    auto ltrim = bind([](std::string s) {
      s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int c) {return !std::isspace(c);}));
      return s;
    });

    auto rtrim = bind([](std::string s) {
      s.erase(std::find_if(s.rbegin(), s.rend(), [](int c) {return !std::isspace(c);}).base(), s.end());
      return s;
    });

    auto split_comma = bind([](std::string input) {
      std::vector<std::string> tokens;
      std::string delimiter = ",";
      size_t pos = 0;
      while ((pos = input.find(delimiter)) != std::string::npos) {
        tokens.push_back(input.substr(0, pos));
        input.erase(0, pos + delimiter.length());
      }
      if (!input.empty())
        tokens.push_back(input);
      return tokens;
    });

    auto csv = ltrim | rtrim | split_comma;

    std::string input = "   a,b,c   ";
    auto tokens = csv(input);
    for (auto t: tokens) {
      std::cout << t << "\n";
    }
  }
}