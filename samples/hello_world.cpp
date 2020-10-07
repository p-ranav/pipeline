#include <pipeline/pipeline.hpp>
using namespace pipeline;
#include <iostream>
#include <map>
#include <string>
#include <vector>

template <class Container, std::size_t... Indices>
auto container_to_tuple_helper(const Container& c, std::index_sequence<Indices...>) {
  return std::make_tuple((*(std::next(c.begin(), Indices)))...);
}
template <size_t N, class Container>
auto to_tuple(Container&& c) {
  return container_to_tuple_helper(std::forward<Container>(c), std::make_index_sequence<N>());
}

template <class F, class Tuple1, class Tuple2, std::size_t... I>
F apply2_impl(F&& f, Tuple1&& t1, Tuple2&& t2, std::index_sequence<I...>)
{
    return (void)std::initializer_list<int>{(std::forward<F>(f)(std::get<I>(std::forward<Tuple1>(t1)), std::get<I>(std::forward<Tuple2>(t2))),0)...}, f;
}
template <class F, class Tuple1, class Tuple2>
constexpr decltype(auto) apply2(F&& f, Tuple1&& t1, Tuple2&& t2)
{
    return apply2_impl(std::forward<F>(f), std::forward<Tuple1>(t1), std::forward<Tuple2>(t2),
                       std::make_index_sequence<std::tuple_size<std::remove_reference_t<Tuple1>>::value>{});
}

int main() {
  auto generate_input = [] { return std::make_tuple("hello ", "world!"); };

  auto title_case = [](std::string input) {
    input[0] = toupper(input[0]);
    return input;
  };

  auto concat = [](auto &&input_1, auto &&input_2) { return std::move(input_1 + input_2); };

  auto print_output = [](auto &&input) { std::cout << input << "\n"; };

  auto pipeline = fn(generate_input) | unzip_into(title_case) | concat | print_output;
  pipeline();

  {    
    auto my_map = std::map<int, std::string> { {5, "a"}, {10, "b"} };
    auto my_tup = to_tuple<2>(my_map);
    apply2([](auto a, auto b) {
      std::cout << a.first << " " << a.second << "\n";
    }, my_tup, my_tup);
    // my_tup is std::tuple<std::pair<int, std::string>, std::pair<int, std::string>>
    // my_tup has value = {{5, "a"}, {10, "b"}}
  }

  {
    auto my_vec = std::vector<float>{1.0f, 2.0f, 3.0f, 4.0f, 5.0f};
    auto my_tup = to_tuple<2>(my_vec);
    apply2([](auto a, auto b) {
      std::cout << a << "\n";
    }, my_tup, my_tup);
    // my_tup is std::tuple<float, float>
    // my_tup has value = {1.0f, 2.0f} 

    auto my_tup_2 = to_tuple<5>(my_vec);
    apply2([](auto a, auto b) {
      std::cout << a << "\n";
    }, my_tup_2, my_tup_2);
    // my_tup_2 is std::tuple<float, float, float, float, float>
    // my_tup_2 has value = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f}
  }
}