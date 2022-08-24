# Facade
There is a small but important difference between the previous design and the new design, namely how lazy functions are used.
In the previous design, if we would define a lazy function as follows:
```cpp
auto deep_thought() {
    // simulate some very expensive calculation
    this_thread::sleep(7500000y);

    return 42;
}

auto lazy_deep_thought = pigro::lazy(deep_thought);
```
We could use it in the following way:
```cpp
assert(     deep_thought() == 42);
assert(lazy_deep_thought() == 42); // same syntax
```
In other words, the api of `deep_thought()` and `lazy_deep_thought()` is identical. If we want to pass either of them to another part of the code and that code is unaware of whether the function is lazy or not, this is totally possible:
```cpp
auto calculate_twice(std::invocable auto f, auto expected) {
    cout << "Calculation started..." << endl;
    assert(f() == expected);
    assert(f() == expected);
    cout << "Calculation finished" << endl;
}

calculate_twice(deep_thought, 42);      // works
calculate_twice(lazy_deep_thought, 42); // works & twice as fast
```
Unfortunately, using the new design, the api of the lazy function is different compared to the original function:
```cpp
assert(     deep_thought()        == 42); // directly returns result
assert(lazy_deep_thought().result == 42); // returns lazy_result
```
Therefore, the following would no longer work:
```cpp
calculate_twice(deep_thought, 42);      // works
calculate_twice(lazy_deep_thought, 42); // syntax error: can't compare lazy_result and int
```
Of course we could fix this in an ad-hoc way:
```cpp
calculate_twice(deep_thought, 42);                               // works
calculate_twice([&] { return lazy_deep_thought().result; }, 42); // now works
```
But that adds quite some noise to the code, especially if this happens in many places.

Fortunately, with a minor change in our new design, we can change the lazy functions, such that when calling them directly, they return the result, whereas when being used as a dependency, they return the `lazy_result` object.

We start by introducing a little helper class, named `facade`, which adapts a `lazy_dependency` such that while calling it, it returns the result directly, instead of the `lazy_result`:
```cpp
template<lazy_dependency F>
struct facade : private F {
    friend auto &ensure_lazy_dependency(facade &self) {
        return static_cast<F &>(self);
    }

    auto operator()() const {
        auto dependency = ensure_lazy_dependency(*this);
        return dependency().result;
    }
};
```
In order to call the original `lazy_dependency`, we first obtain a reference to it by calling `ensure_lazy_dependency()`. This function simply casts the object to a reference of the base class, which is the lazy dependency itself (we are inheriting from it). Later we will see that by cleverly choosing this name (i.e. `ensure_lazy_dependency()`), we manage to get everything working without having to make any more modifications (apart from using the `facade` class).

Now, considering the result of the `lazy()` function is a `lazy_dependency`, we can wrap the result inside of a `facade`:
```cpp
auto lazy(auto f, lazy_dependency auto ...dependencies) {
    auto cache = std::optional<decltype(f(dependencies().result...))>{};

    return facade{[=]() mutable {
        auto is_changed = (dependencies().is_changed || ... || !cache);
        if (is_changed) {
            const auto result = f(dependencies().result...);
            is_changed = result != cache;
            cache = result;
        }

        return lazy_result{is_changed, *cache};
    }};
}
```

With these two changes, we can finally make the previous example work, but without the ad-hoc fix:
```cpp
assert(     deep_thought() == 42); // directly returns result
assert(lazy_deep_thought() == 42); // also directly returns result

calculate_twice(deep_thought, 42);      // works
calculate_twice(lazy_deep_thought, 42); // also works & (still) twice as fast
```

It becomes really interesting though, when we look at a more complex example, like for example the graphics editor example:
```cpp
auto arrow = pigro::lazy(load_image, "arrow.png");
auto mouse_cursor = pigro::lazy(render_mouse_cursor, get_mouse_pos, arrow);
```

Apart from being able to use the results from the lazy functions without having to dig into the `lazy_result` objects:
```cpp
assert(arrow() == image{...});
assert(mouse_cursor() == ui_object{...});
```

we can analyze what is actually happening while constructing the lazy functions `arrow()` and `mouse_cursor()`.

For the `arrow()` lazy function, the _unconstrained_ `lazy()` overload will be selected, because the dependency, `"arrow.png"` does not satisfy the `lazy_dependency` concept.
Looking again at the implementation of that overload:
```cpp
auto lazy(auto f, auto ...dependencies) {
    return lazy(f, ensure_lazy_dependency(dependencies)...);
}
```
we can see that it will transform `"arrow.png"` into a `lazy_dependency` by calling the `ensure_lazy_dependency` function. Subsequently, it will delegate to the main `lazy()` overload, which will put the `facade` around the resulting lazy function, to enable the uniform usage.

The `mouse_cursor()` lazy function is slightly more involved. Also here, because none of the dependencies satisfy the `lazy_dependency` concept, the _unconstrained_ `lazy()` overload is selected. First, we look at the second dependency, `arrow`. Because this dependency _is_ a lazy function, the call to `ensure_lazy_dependency()` will find the one we defined inside of the `facade` class, which in turn returns the unwrapped `lazy_dependency` that returns the `lazy_result`.
The first dependency, however, `get_mouse_pos`, does something interesting. Because of the implementation of the `ensure_lazy_dependency()` that gets called for `get_mouse_pos`:
```cpp
auto ensure_lazy_dependency(std::invocable auto f) {
    return lazy(
        [=](int) { return f(); },
        always_changed
    );
}
```
We don't actually get back something that satisfies the `lazy_dependency` concept, because of the `facade` that is being wrapped around it by the `lazy()` function.
However, it turns out that we don't need any extra modifications to make this work, as it works out of the box: because we are delegating to `lazy()` and not all of the dependencies satisfy the `lazy_dependency` concept, the _unconstrained_ `lazy()` overload is being called _again_. But this time, when the dependencies are being transformed, the first _lazy_ `get_mouse_pos` dependency gets successfully transformed to a `lazy_dependency` (similar to how the `arrow` depedendency was being transformed in the first `lazy()` call). The second dependency, the _lazy_dependency_ `arrow`, will just be passed as-is, due to the `ensure_lazy_dependency()` overload being selected:
```cpp
auto ensure_lazy_dependency(lazy_dependency auto f) {
    return f;
}
```
So after this second round of calls to `ensure_lazy_dependency()`, all of the dependencies now do in fact satisfy the `lazy_dependency` concept, so the final call to `lazy()` will select the main `lazy()` overload.

As a result, for the dependencies, we can arbitrarily mix lazy functions, non-lazy functions (i.e. `std::invocable`), values, as well as functions that already do satisfy the `lazy_dependency` concept (i.e. functions that return a `lazy_result`).
