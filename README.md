# pigro
_Lazy evaluation on steroids_

The Pigro library allows you to define functions in a declarative and reactive way, resulting in code that is easier to reason about, easier to maintain, and less prone to errors.

Let's start out with a simple example. Imagine we have some function that performs a relatively expensive operation. Therefore, you'd want to postpone calling this function  until it is absolutely necessary.

Using the `pigro::lazy` utility, we can very easily create a function that just calls the function the first time it is called and will reuse the previously calculated result any subsequent time it is being called:

```c++
auto long_computation() -> int;

auto lazy_computation = pigro::lazy(long_computation);
```

Behind the scenes, it will cache the return value and will return that for any subsequent call.

Now we can use it as follows:

```c++
auto answer_to_life = lazy_computation(); // may take a while...
assert(answer_to_life == 42);
// ...
auto universe_and_everything = lazy_computation(); // instantaneous!
```

# Features
- [x] easy creation of cached functions 
- [x] reactivity by specifying dependencies
- [x] support for values, functions, and lazy functions as dependency
- [x] skipping re-evaluation if dependencies evaluate to same values as previous ones
- [x] skipping comparison of lazy function dependencies if they were not re-evaluated
- [ ] memory optimization for stateless dependencies
- [ ] memory optimization for dependencies (only store cached result and any stateful transitive dependencies

# Presentations
- [LEVEL UP 2021 Rome Developer Conference](https://github.com/erikvalkering/pigro_presentation_levelup2021/releases/download/v1/presentation.pdf)
