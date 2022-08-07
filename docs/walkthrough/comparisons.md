# Avoiding comparisons
In order to obtain this extra piece of information from a lazy dependency, we need to be able to interact with it in a different way to communicate that we need this extra information. One way to achieve this, is by passing a special marker value as argument while invoking the lazy dependency. In turn, the lazy function can now return a `bool` value indicating whether the function was reevaluated and if so, whether the value was changed, in addition to the evaluation result itself (obtained either from the cache or from reevaluation).

Our new design starts by defining a new *concept*, `pigro::concepts::lazy`:

```cpp
template<typename F>
concept lazy = requires(F f) {
    { f(nullptr).value };
    { f(nullptr).is_changed };
};
```

This roughly means that a function `F` satisfies this concept, if after invoking it using a `nullptr`, it returns some type that has a `value` data member, as well as an `is_changed` data member.
Next, we define the following struct, which holds the computed value of the lazy function, as well as a flag whether it changed:

```cpp
template<typename T, std::same_as<bool> B>
struct lazy_result {
    T value;
    B is_changed;
};
```

Now we can define the modified version of the core algorithm (now renamed to `lazy_core()` for reasons explained later), that is constrained by this new concept and returns the previously defined `lazy_result` struct:

```cpp
auto lazy_core(auto f, concepts::lazy auto ...dependencies) {
    auto cache = std::optional<decltype(f(dependencies(nullptr).value...))>{};

    return [=](std::nullptr_t) mutable {
        auto changed = !cache || (dependencies(nullptr).is_changed || ...);
        if (changed) {
            auto result = f(dependencies(nullptr).value...);
            changed = result != cache;
            cache = result;
        }

        return lazy_result{*cache, changed};
    };
}
```

Because the dependencies are now properly constrained to be `lazy` dependencies, we can invoke them using the `nullptr` argument and first check the `is_changed` flag to see whether they in fact have changed, before calculating the new result. After having calculated the new result, we should still check whether the value has actually changed (which was done in the previous design, but unconditionally). Finally, we store the result in the cache, and return it together with the flag (both combined as a `lazy_result`).

>[!note] In the `pigro` library, the double call to `dependencies(nullptr)` is optimized to a single call.

One big difference with the previous design, apart from the algorithmic improvement (i.e. avoiding comparisons), is that we reduced the memory footprint considerably: each lazy function now needs to store only the cache for its own calculated result, whereas previously, it would also hold the caches for all of its dependencies (even though they were already stored in the dependencies themselves if they were lazy). On top of that, in the previous design, for lazy functions having zero dependencies, we would still have to store an `std::optional<std::tuple<>>`, whereas in the new design it no longer necessary.

Unfortunately, due to the result of the `pigro::lazy_core` function now also satisfying `concepts::lazy`, we are stuck with the following syntax to use it:

```cpp
auto foo(int bar) -> int;
auto bar() -> int;

auto bar_a = pigro::lazy_core(bar);
auto foo_a = pigro::lazy_core(foo, bar_a);

cout << foo_a().value << endl;
```

So let's fix that by adding a nice facade on top of the result of `lazy_core()`:

```cpp
auto facade(concepts::lazy auto f) {
    return [=](auto ...args) mutable {
        if constexpr (sizeof...(args) == 0)
            return f(nullptr).value;
        else
            return f(nullptr);
    };
}

auto lazy(auto f, auto ...dependencies) {
    return facade(lazy_core(f, dependencies...));
}
```

So now the `lazy()` function just wraps the result of the `lazy_core()` function inside of a facade, that either forwards to the lazy function, or forwards to the lazy function **and** "unwraps" the resulting `value`. This should also make it clear why we had to rename the core algorithm to have the `_core` suffix: because otherwise we would have an ambiguity between the facade and the core algorithm.

Now, we are back with the original syntax:

```cpp
auto foo(int bar) -> int;
auto bar() -> int;

auto foo_a = pigro::lazy(foo, bar);

cout << foo_a() << endl;
```

However, this new design assumes that all of the dependencies are lazy dependencies (due to the `concepts::lazy` constraint). Fortunately, this new design is flexible enough to support normal non-lazy function dependencies, as well as value dependencies, which will be discussed in the next section.

We start by defining the following utility function, `lazy_value()`:
```cpp
auto lazy_value(auto value, auto is_changed) {
    return [=](std::nullptr_t) {
        return lazy_result{value, is_changed};
    };
}
```

This function accepts a `value` and an `is_changed` flag, and will create a new function object that satisfies the `concepts::lazy` concept (i.e. invocable using `nullptr` and returns some type with a `value` and `is_changed` data member).

Using this utility function, we can already start supporting the value dependencies use-case, though with some necessary boilerplate:
```cpp
auto foo(int bar) -> int;
auto bar() -> int;

auto foo_c = pigro::lazy(foo, lazy_value(42, false));
```

In here, `lazy_value` creates a lazy dependency that simply always indicates that it did **not** change (due to the `is_changed` parameter having the value `false`).

In order to simplify this syntax, we add back the "syntactic sugar":

```cpp
auto ensure_lazy(concepts::lazy auto dependency) {
    return dependency;
}

auto ensure_lazy(auto dependency) {
    return lazy_value(dependency, false);
}

auto lazy(auto f, auto ...dependencies) {
    return facade(lazy_core(f, ensure_lazy(dependencies)...));
}
```

We've updated the user-facing `lazy()` function, by passing all of the dependencies through the `ensure_lazy()` function overload set. This will ensure that all of the dependencies arriving at the `lazy_core()` function, will satisfy the `concepts::lazy` concept. For dependencies that are already satisfied, we simply return them as-is. For others, we will wrap them with the `lazy_value()` utility, just like we did before.

Now our example becomes:
```cpp
auto foo(int bar) -> int;
auto bar() -> int;

auto foo_c = pigro::lazy(foo, 42);
```

In order to add back support for the final use-case, the normal non-lazy function dependencies, we just have to add a few more overloads to the `ensure_lazy()` overload set:
```cpp
auto ensure_lazy(std::invocable auto dependency) requires concepts::lazy<decltype(dependency)> {
    return dependency;
}

auto ensure_lazy(std::invocable auto dependency) {
    return lazy(
        [=](int) mutable { return dependency(); },
        lazy_value(0, true)
    );
}
```

The first overload is to support lazy functions as dependency that have already gotten the facade treatment, and therefore can be called with and without the `nullptr` argument. The second overload adds support for the actual normal non-lazy function dependencies: it wraps the dependency inside of another function that accepts a single `int`-parameter. This parameter in turn is provided by wrapping this function using a call to `pigro::lazy()`, and using the `lazy_value()` utility to create a dependency that returns `0` and always indicates it has changed, which is exactly the behaviour we had in the previous version.

So now, we have added back support for all three use-cases:
```cpp
auto foo(int bar) -> int;
auto bar() -> int;

auto foo_a = pigro::lazy(foo, bar);
auto foo_b = pigro::lazy(foo, foo_a);
auto foo_c = pigro::lazy(foo, 42);
```

However, the underlying implementation is much more efficient, both from a work as well as memory perspective.
