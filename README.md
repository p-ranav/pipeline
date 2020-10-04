# pipeline

`pipeline` is a library that lets you setup data processsing pipelines like this:

```cpp
#include <pipeline/pipeline.hpp>
using namespace pipeline;

int main() {
  auto generate_input = fn([] { return std::make_tuple(158, 33); });

  auto double_it = fn([](auto a, auto b) { return std::make_tuple(a * 2, b * 2); });

  auto sum = fn([](auto a, auto b) { return a + b; });
  auto diff = fn([](auto a, auto b) { return a - b; });

  auto print_results = fn([](auto sum, auto diff) {
    std::cout << "Sum = " << sum << ", Diff = " << diff << std::endl;
  });

  auto pipeline = generate_input | double_it | fork(sum, diff) | print_results;
  pipeline();
}

// prints:
// Sum = 382, Diff = 250
```
