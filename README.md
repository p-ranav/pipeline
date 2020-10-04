# pipeline

## Getting Started

`pipeline` is a `C++17` header-only template library that lets you setup data processsing pipelines. Simply include `<pipeline/pipeline.hpp>` and you're good to go. There is also a single_include version in `single_include/`.

Here's a quick-start example for this library:

Here, we construct the following pipeline:

```cpp
auto pipeline = generate_input | double_it | fork_async(sum, diff) | print_results;
```

Use `pipeline::fn` to wrap your functions and use `operator|` to chain these functions together into a pipeline. Constructs like `fork` and `unzip` can be used to setup concurrency / parallel processing of inputs. 

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

In the above code,
* The output of `generate_input`, i.e., a tuple of ints, is unpacked and passed to the next stage in the pipeline.
* Forked segments, i.e., `sum` and `diff`, run in parallel - futures are passed along to the next stage in the pipeline.
