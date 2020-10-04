# pipeline

`pipeline` is a library that lets you setup data processsing pipelines like this:

```cpp
#include <pipeline/pipeline.hpp>
using namespace pipeline;
#include <iostream>

int main() {
  auto generate_input = fn([] { return std::make_tuple(158, 33); });

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
// Sum = 382, Diff = 250
```

In the above code, the forked segments, i.e., `sum` and `diff`, run in parallel - futures are passed along to the next stage in the pipeline.
