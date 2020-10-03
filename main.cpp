#include <pipeline/pipeline.hpp>
#include <fstream>
#include <optional>
#include <sstream>
#include <functional>
#include <chrono>
using namespace pipeline;

int main() {
 
  {
    auto add = fn(std::bind([](int a, int b) { return a + b; }, 5, 10));
    auto square = fn([](int a) { return a * a; });
    auto pretty_print_square = fn([](int result, std::string msg = "Result = ") { std::cout << msg << std::to_string(result) << "\n"; });

    auto pipeline = add | square | pretty_print_square;
    pipeline();
  }

  // Using `pipe(...)` function
  {
    auto add = fn([](int a, int b) { return a + b; });
    auto square = fn([](int a) { return a * a; });
    auto pretty_print_square = fn([](int result, std::string msg = "Result = ") { std::cout << msg << std::to_string(result) << "\n"; });

    auto pipeline = pipe(add, square, pretty_print_square);
    pipeline(15, 23);
  }

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
  // and call next bind
  {
    auto square = fn([](int a) { return a * a; });
    auto cube = fn([](int a) { return a * a * a; });

    auto pipeline = 
      square
      | fork(square, cube)
      | fn([](auto square_result, auto cube_result) { 
          std::cout << square_result << " " << cube_result << "\n"; 
        });
    pipeline(5);
  }

  // Unpack fork result (which is a tuple) 
  // and call next bind
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
  // and call next bind
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

  // Keep fork result as a tuple
  // and call next bind
  {
    auto add_10 = fn([](int a) { return a + 10; });
    auto square = fn([](int a) { return a * a; });
    auto cube = fn([](int a) { return a * a * a; });
    auto pipeline = 
      square
      | fork((add_10 | square), square, cube)
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
    auto counter = fn([](auto n) { 
      static size_t previous{0};
      auto result = std::make_tuple(n, previous);
      previous = n;
      return result;
    });

    auto print_n = fn([](auto n, auto prev) {
      std::cout << "N = " << n << "\n";
      return n;
    });

    auto print_prev = fn([](auto n, auto prev) {
      std::cout << "Prev = " << prev << "\n";
      return prev;
    });

    auto print_result = fn([](auto n, auto prev) { 
      std::cout << "Stateful result = " << n << " - " << prev << "\n"; 
    });

    auto print_result_2 = fn([](auto packed) {
      std::cout << "Stateful result = " << std::get<0>(packed) << " - " << std::get<1>(packed) << "\n"; 
    });

    auto pipeline = counter | fork(print_n, print_prev) | print_result;
    pipeline(5);
    pipeline(10);
    pipeline(15);
  }

  {
    auto read_file = fn([](std::string_view filename = "main.cpp") {
      std::string buffer;

      std::ifstream file(filename);
      file.seekg(0, std::ios::end);
      buffer.resize(file.tellg());
      file.seekg(0);
      file.read(buffer.data(), buffer.size());

      return buffer;
    });

    auto read_next_line = fn([](auto contents) -> std::optional<std::string> {
      static std::istringstream f(contents);
      std::string line;    
      if (std::getline(f, line)) {
        return line;
      }
      return {};
    });

    auto print_line = fn([](auto line) { if (line.has_value()) std::cout << line.value() << "\n"; });

    auto line_printer = read_file | read_next_line | print_line;

    line_printer();
    line_printer();
    line_printer();
  }

  {
    auto t1 = fn([]() { std::cout << "TaskA\n"; });
    auto t2 = fn([]() { std::cout << "TaskB\n"; });
    auto t3 = fn([]() { std::cout << "TaskC\n"; });
    auto t4 = fn([]() { std::cout << "TaskD\n"; });
    auto pipeline = t1 | fork(t2, t3) | t4;
    pipeline();
  }

  {
    auto ltrim = fn([](std::string s) {
      s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int c) {return !std::isspace(c);}));
      return s;
    });

    auto rtrim = fn([](std::string s) {
      s.erase(std::find_if(s.rbegin(), s.rend(), [](int c) {return !std::isspace(c);}).base(), s.end());
      return s;
    });

    auto split_comma = fn([](std::string input) {
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

  // Fork with async
  {
    auto factorial_async = [](auto n) {
      return std::async(std::launch::async, [](auto n) {
        decltype(n) result{1};
        for(size_t i = 1; i <= n; ++i) {
          result *= i;
        }
        return result;
      }, n);
    };

    auto print_result = fn([](auto&& result) {
      std::cout << std::get<0>(result).get() << " " << std::get<1>(result).get() << "\n";
    });

    auto pipeline = fork(fn(std::bind(factorial_async, 5)), fn(std::bind(factorial_async, 10))) | print_result;
    pipeline();
    // should print:
    // 120 3628800
  }

  {
    auto greet = fn([]() { std::cout << "Hello World!\n"; });
    auto pair = fork_async(greet, greet, greet);
    pair();
  }

  // {
  //   bind t1 = []() { std::cout << "TaskA\n"; };
  //   bind t2 = []() { return 1; };
  //   bind t3 = []() { return 2; };
  //   bind t4 = [](auto&& t1, auto&& t2) { 
  //     std::cout << t1.get() << ", " << t2.get() << "\n";
  //     std::cout << "TaskD\n";
  //   };
  //   auto pipeline = t1 | fork_async(t2, t3) | t4;
  //   pipeline();
  // }

  std::cout << "\n\n";

  {
    auto t1 = fn([] { std::cout << "TaskA\n"; });
    auto t2 = fn([] {
      std::cout << "Running TaskB\n";
      return 2; 
    });
    auto t3 = fn([] { 
      std::cout << "Running TaskC\n";
      return 3.14f; 
    });
    auto t4 = fn([] { 
      std::cout << "Running TaskD\n";
      return "Hello World"; 
    });
    auto t5 = fn([](auto t2_result, auto t3_result, auto t4_result) {
      std::cout << "TaskB: " << t2_result << "\n";
      std::cout << "TaskC: " << t3_result << "\n";
			std::cout << "TaskD: " << t4_result << "\n";
      
      std::cout << "TaskE\n";
    });
    
    auto pipeline = t1 | fork(t2, t3, t4) | t5;
    pipeline();
 }

   {
    auto t1 = fn([]() { std::cout << "TaskA\n"; });
    auto t2 = fn([]() { 
      std::cout << "Running TaskB\n";
      return 2; 
    });
    auto t3 = fn([]() { 
      std::cout << "Running TaskC\n";
      return 3.14f; 
    });
    auto t4 = fn([]() { 
      std::cout << "Running TaskD\n";
      return "Hello World"; 
    });
    auto t5 = fn([](auto&& t2_future, auto&& t3_future, auto&& t4_future) {
      auto t2_result = t2_future.get();
      std::cout << "TaskB: " << t2_result << "\n";

      auto t3_result = t3_future.get();
      std::cout << "TaskC: " << t3_result << "\n";

      auto t4_result = t4_future.get();
			std::cout << "TaskD: " << t4_result << "\n";
      
      std::cout << "TaskE\n";
    });
    
    auto pipeline = t1 | fork_async(t2, t3, t4) | t5;
    pipeline();
 }

  // Ignore result of previous stage in pipeline
 {
   auto add = fn([](int a, int b) { return a + b; });
   auto print_result = fn([](int sum) { std::cout << "Sum = " << sum << "\n"; });

   auto pipeline_1 = add | print_result;
   pipeline_1(4, 9);

   auto greet = fn([]() { std::cout << "Discarding args from previous stage\n"; });
   auto pipeline_2 = add | greet;
   pipeline_2(6, 11);
 }

 {
  // Dynamic tasking and subflows
  auto A = fn([](){ std::cout << "A" << std::endl; });
  auto C = fn([](){ std::cout << "C\n"; });
  auto D = fn([](){ std::cout << "D\n"; });

  auto B = fn([](){
    auto B1 = fn([](){ std::cout << "B1\n"; });
    auto B2 = fn([](){ std::cout << "B2\n"; });
    auto B3 = fn([](){ std::cout << "B3\n"; });

    auto B = fork(B1, B2) | B3;
    return B();
  });

  auto pipeline = A | fork(C, B) | D;
  pipeline();
 }

 {
   auto doubler = fn([](int a) { return a * 2; });
   std::deque<int> queue;

   auto sink = fn([&queue](int a) mutable {
     std::cout << a << "\n";
     queue.push_back(a);
     return a;
   });

   auto pipeline = doubler | sink | doubler | sink;

   pipeline(1);
   pipeline(2);
   pipeline(3);
   pipeline(4);
   pipeline(5);

   for (auto& q: queue) {
     std::cout << q << " ";
   }
   std::cout << "\n";

 }

 {
   auto doubler = fn([](int a) { return a * 2; });
   std::deque<int> queue;

   auto sink = fn([&queue](int a) mutable {
     std::cout << a << "\n";
     queue.push_back(a);
     return a;
   });

   auto pipeline = pipe(doubler, sink, doubler, sink);

   pipeline(1);
   pipeline(2);
   pipeline(3);
   pipeline(4);
   pipeline(5);

   for (auto& q: queue) {
     std::cout << q << " ";
   }
   std::cout << "\n";

 }

 {
   // auto pipeline = foo | unzip_into(fork(f1, f2)) | print_results;

   // output of foo: tuple(i_1, i_2, i_3)
   // unzip_into(foo) will need to become:
   // -> fork(bind(foo_1, i_1), bind(foo_2, i_2), bind(foo_3, i_3))

   auto make_args = fn([] { return std::make_tuple(1, std::string{"Hello"}); });
   auto f1 = fn([](int a) { return a * a; });
   auto f2 = fn([](std::string s) { return s + ", World!"; });

   auto print_result = fn([] (auto f1_result, auto f2_result) { 
     std::cout << f1_result << ", " << f2_result << "\n";
     std::cout << "Done\n"; 
   });

   auto pipeline = make_args | unzip_into(f1, f2) | print_result;
   pipeline();
 }

 {
   // run parallel and unzip_into | unzip_into

   auto make_args = fn([] { return std::make_tuple(1, std::string{"Hello"}); });
   auto f1 = fn([](int a) { return a * a; });
   auto f2 = fn([](std::string a) { return a + ", World!"; });
   auto print_f1_f2_results = fn([](int a, std::string b) {
     std::cout << a << ", " << b << "\n";
   });

   auto print_f1_result = fn([](auto a) { std::cout << "f1 = " << a << "\n"; });
   auto print_f2_result = fn([](auto a) { std::cout << "f2 = " << a << "\n"; });

   auto pipeline = 
      make_args | unzip_into(f1, f2) | fork(print_f1_f2_results, unzip_into(print_f1_result, print_f2_result))
      | fn([] { std::cout << "Done\n"; });
   pipeline();

 }

  // Move unique_ptr through a pipeline
 {

   auto f1 = fn([]() { return std::unique_ptr<int>(new int(5)); });
   auto f2 = fn([](auto&& ptr) { return std::move(ptr); });
   auto pipeline = f1 | f2;
   auto ptr = pipeline();
   std::cout << *ptr << "\n";
 }

  // Mutating a reference
 {
   auto f1 = fn([](int& ref) -> int& { ref = ref * 2; return ref; });
   auto f2 = fn([](int& ref) { ref = ref + 3; });

   auto pipeline = f1 | f2;
   int foo = 5;
   // f1(foo);
   // f2(foo);
   pipeline(foo);
   std::cout << "Foo after mutation: " << foo << "\n";

 }

  // Mutating a reference with a bind for scaling factor
 {
   auto f1 = fn(std::bind([](int& ref, int scale) -> int& { ref = ref * scale; return ref; }, std::placeholders::_1, std::placeholders::_2));
   auto f2 = fn([](int& ref) { ref = ref + 3; });

   auto pipeline = f1 | f2;
   int foo = 5;
   // f1(foo);
   // f2(foo);
   pipeline(foo, 2);
   std::cout << "Foo after mutation: " << foo << "\n";

 }

  // Mutating a vector
 {
   std::vector<int> numbers{1, 2, 3, 4, 5};

   auto doubler = [](auto& numbers) -> auto& {
     std::transform(numbers.begin(), numbers.end(), numbers.begin(), [](auto n) { return n * 2; });
     return numbers;
   };

   auto square = [](auto& numbers) -> auto& {
    std::transform(numbers.begin(), numbers.end(), numbers.begin(), [](auto n) { return n * n; });
    return numbers;
   };

   auto pipeline = pipeline::from(doubler) | square | doubler | square;
   pipeline(numbers);

   for (const auto& n : numbers) {
     std::cout << n << " ";
   }
   std::cout << "\n";

 }
}