# Caching
In the previous example we wrapped a function that did not return any value. Often, however, functions do return something useful. Consider for example a function that performs a relatively expensive calculation but also returns the result from that calculation. In order to support this use case, the `pigro::lazy()` utility will cache any value returned by the wrapped function. Any subsequent time that the function is called, it simply returns the previously-cached value:

```cpp
auto deep_thought() {
    // simulate some very expensive calculation
    this_thread::sleep(7500000y);

    return 42;
}

auto lazy_deep_thought = pigro::lazy(deep_thought);

auto answer_to_life = lazy_deep_thought(); // may take a while...
assert(answer_to_life == 42);

auto universe_and_everything = lazy_deep_thought(); // instantaneous!
assert(universe_and_everything == 42);
```

As before, a **heavily simplified** version of the `pigro::lazy()` function might give a better understand of what is going on behind the scenes:
```cpp
auto lazy(auto f) {
    auto cache = std::optional<decltype(f())>{};

    return [=]() mutable {
        if (!cache)
            cache = f();

        return *cache;
    };
}
```

Basically, the only change we made in order to support caching, was to change the `bool` flag to an `std::optional`, which holds both the cached value and the flag.
