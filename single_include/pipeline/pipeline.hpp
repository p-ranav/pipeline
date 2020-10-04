#pragma once
#include <tuple>

namespace pipeline {

template <typename Fn>
class fn;

template <typename T1, typename T2>
class pipe_pair;

template <typename Fn, typename... Fns>
class fork;

template <typename Fn, typename... Fns>
class fork_async;

template <typename Fn, typename... Fns>
class unzip_into;

template <typename Fn, typename... Fns>
class unzip_into_async;

namespace details {

// is_tuple constexpr check
template <typename> struct is_tuple: std::false_type {};
template <typename... T> struct is_tuple<std::tuple<T...>>: std::true_type {};

template <typename Test, template <typename...> class Ref>
struct is_specialization : std::false_type {};

template <template <typename...> class Ref, typename... Args>
struct is_specialization<Ref<Args...>, Ref> : std::true_type {};

template <class F, size_t... Is>
constexpr auto index_apply_impl(F f, std::index_sequence<Is...>) {
  return f(std::integral_constant<size_t, Is> {}...);
}

template <size_t N, class F>
constexpr auto index_apply(F f) {
  return  index_apply_impl(f, std::make_index_sequence<N>{});
}

// Unpacks Tuple into a parameter pack
// Calls f(parameter_pack)
template <class Tuple, class F>
constexpr auto apply(Tuple t, F f) {
  return index_apply<std::tuple_size<Tuple>{}>(
    [&](auto... Is) { return f(std::get<Is>(t)...); }
  );
}

template <size_t N, typename T>
constexpr decltype(auto) make_repeated_tuple(T t) {
  if constexpr (N == 1) {
    return t;
  } else {
    return make_repeated_tuple<N - 1>(std::tuple_cat(std::make_tuple(std::get<0>(t)), t));
  }
}

template <typename F, typename... Args>
constexpr bool is_invocable_on() {
  if constexpr (details::is_specialization<typename std::remove_reference<F>::type, fn>::value) {
    // F is an `fn` type
    return std::remove_reference<F>::type::template is_invocable_on<Args...>();
  }
  else if constexpr (
    details::is_specialization<typename std::remove_reference<F>::type, pipe_pair>::value ||
    details::is_specialization<typename std::remove_reference<F>::type, fork>::value ||
    details::is_specialization<typename std::remove_reference<F>::type, fork_async>::value ||
    details::is_specialization<typename std::remove_reference<F>::type, unzip_into>::value ||
    details::is_specialization<typename std::remove_reference<F>::type, unzip_into_async>::value
  ) {
    return is_invocable_on<typename F::left_type, Args...>();
  }
  else {
    return std::is_invocable<F, Args...>::value;
  }
}

}

}
#pragma once
// #include <pipeline/details.hpp>

namespace pipeline {

template <typename Fn>
class fn {
  Fn fn_;

public:
  fn(Fn fn): fn_(fn) {}

  template <typename... T>
  decltype(auto) operator()(T&&... args) {
    return fn_(args...);
  }

  template <typename... A>
  static constexpr bool is_invocable_on() {
    return std::is_invocable<Fn, A...>::value;
  }

  template <typename T>
  auto operator|(T&& rhs) {
    return pipe_pair<fn<Fn>, T>(*this, std::forward<T>(rhs));
  }
};

}
#pragma once
// #include <pipeline/details.hpp>

namespace pipeline {

template <typename T1, typename T2>
class pipe_pair {
  T1 left_;
  T2 right_;

public:
  typedef T1 left_type;
  typedef T2 right_type;

  pipe_pair(T1 left, T2 right) : left_(left), right_(right) {}

  template <typename... T>
  decltype(auto) operator()(T&&... args) {
    typedef typename std::result_of<T1(T...)>::type left_result_type;

    // check if left_ result is a tuple
    if constexpr (details::is_tuple<left_result_type>::value) {
      // left_ result is a tuple

      if constexpr (details::is_invocable_on<T2, left_result_type>()) {
        // right_ takes a tuple
        return right_(left_(std::forward<T>(args)...));
      } else {
        // check if right is invocable without args
        if constexpr (details::is_invocable_on<T2>()) {
          left_(std::forward<T>(args)...);
          return right_();
        } else {
          // unpack tuple into parameter pack and call right_
          return details::apply(left_(std::forward<T>(args)...), right_);
        }
      }
    } else {
      // left_result_type not a tuple
      // call right_ with left_result
      if constexpr (!std::is_same<left_result_type, void>::value) {
        // if right can be invoked without args
        // just call without args
        if constexpr (details::is_invocable_on<T2>()) {
          left_(std::forward<T>(args)...);
          return right_();
        } else {
          return right_(left_(std::forward<T>(args)...));
        }
      } else {
        // left result is void
        left_(std::forward<T>(args)...);
        return right_();
      }
    }
  }

  template <typename T3>
  auto operator|(T3&& rhs) {
    return pipe_pair<pipe_pair<T1, T2>, T3>(*this, std::forward<T3>(rhs));
  }
};

template <typename T1, typename T2>
auto pipe(T1&& t1, T2&& t2) {
  return pipe_pair<T1, T2>(std::forward<T1>(t1), std::forward<T2>(t2));
}

template <typename T1, typename T2, typename... T>
auto pipe(T1&& t1, T2&& t2, T&&... args) {
  return pipe(pipe<T1, T2>(std::forward<T1>(t1), std::forward<T2>(t2)), std::forward<T>(args)...);
}

}
#pragma once
// #include <pipeline/details.hpp>
// #include <pipeline/fn.hpp>
#include <thread>
#include <future>
#include <functional>

namespace pipeline {

template <typename Fn, typename... Fns>
class fork {
  std::tuple<Fn, Fns...> fns_;

  // Takes a tuple of arguments          : args_tuple
  // Takes a parameter pack of functions : fns
  //
  // Converts the args_tuple into a parameter pack
  // Calls std::async and spawns thread, one for each function
  // Each thread will run the function with the parameter pack of arguments
  //
  // Waits for all the threads to finish - we will get a tuple of futures
  // Returns a tuple of future.get() values
  // For functions that return `void`, we simply return true to indicate that
  // the function completed
  template <typename A, typename... T>
  auto do_async_await(A&& args_tuple, T&&... fns) {
    return fn(std::bind([](A args_tuple, T... fns) {

      auto unpack = [](auto tuple, auto fn) {
        return details::apply(tuple, fn);
      };

      auto futures = std::make_tuple(std::async(std::launch::async | std::launch::deferred, unpack, args_tuple, fns)...);

      // wait on all futures and return a tuple of results (each forked job)
      auto join = [](auto&&... future) {

        auto join_one = [](auto&& future) {
          typedef decltype(future.get()) future_result;
          if constexpr (std::is_same<future_result, void>::value) {
            // future.get() will return void
            // return true instead to indicate completion of function call
            return true;
          } else {
            return future.get();
          }
        };

        return std::make_tuple((join_one(future))...);
      };
      return details::apply(std::move(futures), join);
    }, std::forward<A>(args_tuple), std::forward<T>(fns)...));
  }

public:
  typedef Fn left_type;

  fork(Fn first, Fns... fns) : fns_(first, fns...) {}

  template <typename... Args>
  decltype(auto) operator()(Args&&... args) {
    // We have a tuple of functions to run in parallel           - fns_
    // We have a parameter pack of args to pass to each function - args...
    // 
    // The following applies a lambda function to the fns_ tuple
    // the fns_ tuple is converted into a parameter pack and 
    // the lambda is called with that parameter pack
    // 
    // Then, we call do_async_await which takes a tuple of args
    // along with the parameter pack of functions
    return details::apply(fns_, [this, args...](auto... fns) {
      return do_async_await(std::make_tuple(args...), fns...)();
    });
  }

  template <typename T3>
  auto operator|(T3&& rhs) {
    return pipe_pair<fork<Fn, Fns...>, T3>(*this, std::forward<T3>(rhs));
  }

};

}
#pragma once
// #include <pipeline/details.hpp>
// #include <pipeline/fn.hpp>
#include <thread>
#include <future>
#include <functional>

namespace pipeline {

// Very similar to fork
// Unlike fork, fork_async::operator() does not wait for results
// - it simply returns a tuple of futures
template <typename Fn, typename... Fns>
class fork_async {
  std::tuple<Fn, Fns...> fns_;

  // Takes a tuple of arguments          : args_tuple
  // Takes a parameter pack of functions : fns
  //
  // Converts the args_tuple into a parameter pack
  // Calls std::async and spawns thread, one for each function
  // Each thread will run the function with the parameter pack of arguments
  //
  // Waits for all the threads to finish - we will get a tuple of futures
  // Returns a tuple of future.get() values
  // For functions that return `void`, we simply return true to indicate that
  // the function completed
  template <typename A, typename... T>
  auto do_fork_async(A&& args_tuple, T&&... fns) {
    return fn(std::bind([](A args_tuple, T... fns) {

      auto unpack = [](auto tuple, auto fn) {
        return details::apply(tuple, fn);
      };

      return std::make_tuple(std::async(std::launch::async | std::launch::deferred, unpack, args_tuple, fns)...);
    }, std::forward<A>(args_tuple), std::forward<T>(fns)...));
  }

public:
  typedef Fn left_type;

  fork_async(Fn first, Fns... fns) : fns_(first, fns...) {}

  template <typename... Args>
  decltype(auto) operator()(Args&&... args) {
    // We have a tuple of functions to run in parallel           - fns_
    // We have a parameter pack of args to pass to each function - args...
    // 
    // The following applies a lambda function to the fns_ tuple
    // the fns_ tuple is converted into a parameter pack and 
    // the lambda is called with that parameter pack
    // 
    // Then, we call do_fork_async which takes a tuple of args
    // along with the parameter pack of functions
    return details::apply(fns_, [this, args...](auto... fns) {
      return do_fork_async(std::make_tuple(args...), fns...)();
    });
  }

  template <typename T3>
  auto operator|(T3&& rhs) {
    return pipe_pair<fork_async<Fn, Fns...>, T3>(*this, std::forward<T3>(rhs));
  }

};

}
#pragma once
// #include <pipeline/details.hpp>
// #include <pipeline/fn.hpp>
#include <thread>
#include <future>
#include <functional>

namespace pipeline {

template <typename Fn, typename... Fns>
class unzip_into {
  std::tuple<Fn, Fns...> fns_;

  template <class F, class Tuple1, class Tuple2, std::size_t... I>
  auto apply2_impl(F&& f, Tuple1&& t1, Tuple2&& t2, std::index_sequence<I...>) {
    return fork((std::forward<F>(f)(std::get<I>(std::forward<Tuple1>(t1)), std::get<I>(std::forward<Tuple2>(t2))))...);
  }

  template <class F, class Tuple1, class Tuple2>
  constexpr decltype(auto) apply2(F&& f, Tuple1&& t1, Tuple2&& t2) {
    return apply2_impl(std::forward<F>(f), std::forward<Tuple1>(t1), std::forward<Tuple2>(t2),
                      std::make_index_sequence<std::tuple_size<std::remove_reference_t<Tuple1>>::value>{});
  }

public:
  typedef Fn left_type;

  unzip_into(Fn first, Fns... fns) : fns_(first, fns...) {}

  template <typename... Args>
  decltype(auto) operator()(Args&&... args) {
    // We have a tuple of functions to run in parallel - fns_
    // We have a parameter pack of args to UNZIP 
    // and then pass to each function - args...
    const auto bind_arg = [](auto&& fn, auto&& arg) {
      return pipeline::fn(std::bind(fn, std::move(arg)));
    };

    if constexpr (std::tuple_size<std::tuple<Fn, Fns...>>::value == sizeof...(args)) {
      // sizeof(tuple of fns) == sizeof(args)
      // 1-1 mapping

      auto unzipped_fork = apply2(bind_arg, fns_, std::tuple<Args...>(std::forward<Args>(args)...));

      // Let's say the input args were (arg1, arg2, arg3, ...)
      // And we have functions (fn1, fn2, fn3, ...)

      // The above code has constructed a fork:
      // fork(bind(fn1, arg1), bind(fn2, arg2), bind(fn3, arg3), ...)
      //
      // Each function is bound to an argument from the input
      // and because it is a fork, these can/will run in parallel

      // Simply run the fork and return the results
      return unzipped_fork();
    }
    else if constexpr (std::tuple_size<std::tuple<Fn, Fns...>>::value == 1) {
      // sizeof(tuple of fns) = 1
      // Unpack the args on the left and pass each arg to the same function
      // Essentially, we're calling the same function on each of the args
      // Useful if each arg in the tuple is the same type and we're doing the same operation

      // First, we make a tuple of functions of size = sizeof...(args)
      // From one function `fn`, we get: tuple {fn, fn, fn, fn, ...}
      // Then we have arguments (arg1, arg2, arg3, ....)

      // We construct the fork:
      // fork(bind(fn, arg1), bind(fn, arg2), bind(fn, arg3), ...)
      //
      // Each function (the same one in this case) is bound to an argument from the input
      // and because it is a fork, these can/will run in parallel
      //
      // Once the fork is constructed, simply run the fork

      auto repeated_tuple_fn = details::make_repeated_tuple<sizeof...(args)>(std::make_tuple(std::get<0>(fns_)));
      auto unzipped_fork = apply2(bind_arg, repeated_tuple_fn, std::tuple<Args...>(std::forward<Args>(args)...));
      return unzipped_fork();
    }
    else {
      static_assert(std::tuple_size<std::tuple<Fn, Fns...>>::value == sizeof...(args));
    }
  }

  template <typename T3>
  auto operator|(T3&& rhs) {
    return pipe_pair<unzip_into<Fns...>, T3>(*this, std::forward<T3>(rhs));
  }

};

}
#pragma once
// #include <pipeline/details.hpp>
// #include <pipeline/fn.hpp>
#include <thread>
#include <future>
#include <functional>

namespace pipeline {

template <typename Fn, typename... Fns>
class unzip_into_async {
  std::tuple<Fn, Fns...> fns_;

  template <class F, class Tuple1, class Tuple2, std::size_t... I>
  auto apply2_impl(F&& f, Tuple1&& t1, Tuple2&& t2, std::index_sequence<I...>) {
    return fork_async((std::forward<F>(f)(std::get<I>(std::forward<Tuple1>(t1)), std::get<I>(std::forward<Tuple2>(t2))))...);
  }

  template <class F, class Tuple1, class Tuple2>
  constexpr decltype(auto) apply2(F&& f, Tuple1&& t1, Tuple2&& t2) {
    return apply2_impl(std::forward<F>(f), std::forward<Tuple1>(t1), std::forward<Tuple2>(t2),
                      std::make_index_sequence<std::tuple_size<std::remove_reference_t<Tuple1>>::value>{});
  }

public:
  typedef Fn left_type;

  unzip_into_async(Fn first, Fns... fns) : fns_(first, fns...) {}

  template <typename... Args>
  decltype(auto) operator()(Args&&... args) {
    // We have a tuple of functions to run in parallel  - fns_
    // We have a parameter pack of args to UNZIP 
    // and then pass to each function - args...
    const auto bind_arg = [](auto&& fn, auto&& arg) {
      return pipeline::fn(std::bind(fn, std::move(arg)));
    };

    if constexpr (std::tuple_size<std::tuple<Fn, Fns...>>::value == sizeof...(args)) {
      // sizeof(tuple of fns) == sizeof(args)
      // 1-1 mapping

      auto unzipped_fork_async = apply2(bind_arg, fns_, std::tuple<Args...>(std::forward<Args>(args)...));

      // Let's say the input args were (arg1, arg2, arg3, ...)
      // And we have functions (fn1, fn2, fn3, ...)

      // The above code has constructed a fork:
      // fork(bind(fn1, arg1), bind(fn2, arg2), bind(fn3, arg3), ...)
      //
      // Each function is bound to an argument from the input
      // and because it is a fork, these can/will run in parallel

      // Simply run the fork and return the results
      return unzipped_fork_async();
    }
    else if constexpr (std::tuple_size<std::tuple<Fn, Fns...>>::value == 1) {
      // sizeof(tuple of fns) = 1
      // Unpack the args on the left and pass each arg to the same function
      // Essentially, we're calling the same function on each of the args
      // Useful if each arg in the tuple is the same type and we're doing the same operation

      // First, we make a tuple of functions of size = sizeof...(args)
      // From one function `fn`, we get: tuple {fn, fn, fn, fn, ...}
      // Then we have arguments (arg1, arg2, arg3, ....)

      // We construct the fork:
      // fork(bind(fn, arg1), bind(fn, arg2), bind(fn, arg3), ...)
      //
      // Each function (the same one in this case) is bound to an argument from the input
      // and because it is a fork, these can/will run in parallel
      //
      // Once the fork is constructed, simply run the fork

      auto repeated_tuple_fn = details::make_repeated_tuple<sizeof...(args)>(std::make_tuple(std::get<0>(fns_)));
      auto unzipped_fork_async = apply2(bind_arg, repeated_tuple_fn, std::tuple<Args...>(std::forward<Args>(args)...));
      return unzipped_fork_async();
    }
    else {
      static_assert(std::tuple_size<std::tuple<Fn, Fns...>>::value == sizeof...(args));
    }
  }

  template <typename T3>
  auto operator|(T3&& rhs) {
    return pipe_pair<unzip_into<Fns...>, T3>(*this, std::forward<T3>(rhs));
  }

};

}
