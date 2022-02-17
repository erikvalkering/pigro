# Avoiding comparisons
Consider again a somewhat simpler version of the previous code fragment, in which we made the arrow icon also lazy:
```cpp
auto render_mouse_cursor(const point_2d pos, const image &icon) -> ui_object;
auto get_mouse_pos() -> point_2d;
auto load_image(const std::string_view filename) -> image;

auto arrow = pigro::lazy(load_image, "arrow.png");
auto mouse_cursor = pigro::lazy(render_mouse_cursor, get_mouse_pos, arrow);

// Rendering loop for a graphical editor
while (true) {
    mouse_cursor();
}
```

There is a subtle performance issue here: if we don't move the mouse, we don't need to redraw the mouse cursor. You'd therefore expect the call to `mouse_cursor()` to be a very cheap operation. Unfortunately, with the current design of the `pigro::lazy()` function, this is far from cheap. Here is again the core algorithm of the current design:
```cpp
auto lazy(auto f, std::invocable auto ...dependencies) {
    auto cache = std::optional<decltype(f())>{};
    auto dependencies_cache = std::optional<decltype(std::tuple{dependencies()...})>{};

    return [=]() mutable {
        const auto args = std::tuple{dependencies()...};
        if (!cache || args != dependencies_cache) {
            cache = std::apply(f, args);
            dependencies_cache = args;
        }

        return *cache;
    };
}
```

As can be seen, in order to determine whether we need to reevaluate the lazy function, we are **unconditionally** comparing *all* of the evaluations of the dependencies with those previously cached (i.e. the `args != dependencies_cache` bit), even if they did **not** change. Even though the *evaluation* of the `arrow` dependency is very cheap (i.e. it returns the cached value instead of reloading the image), the comparison is not cheap at all: it needs to perform a comparison between two `image` objects each time before `mouse_cursor()` realizes nothing needs to be done.

What we need, is a way to tell whether the value returned from the dependency was newly computed, or not, in which case we don't need to perform the comparison. If the dependency was a simple function, it will always be recomputed. Only if the dependency is a lazy function (or a value, which are converted into lazy functions), the computed value will be cached within the lazy function. Therefore, we should only consider this latter case: dependencies that are lazy.

In order to obtain this extra piece of information from a lazy dependency, we need to be able to interact with it in a different way to make it clear that we need this extra information, for example by passing a special marker value as argument to the lazy dependency. In turn, the lazy function can now return a `bool` value indicating whether the function was reevaluated, in addition to the evaluation result itself (obtained either from the cache or from reevaluation).

Furthermore, another observation that can be made, is that there is some state duplication going on now: for each lazy dependency, the evaluation result is stored in both the `dependencies_cache` of the lazy function, as well as in the `cache` of the lazy dependency. On top of that, for each lazy dependency, if it doesn't have any dependencies, it still stores a `std::optional<std::tuple<>>`, which is rather useless. Finally, the only real use for the `dependencies_cache`, is that for the non-lazy function dependencies, for which the previously evaluated results need to be compared in order to determine whether the lazy function needs to be reevaluated. Fortunately, the new design that will be discussed next, will tackle all of these issues, while still allowing the final use case for non-lazy function dependencies.

```cpp
auto lazy_core(auto f, lazy_function auto ...dependencies) {
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

>[!note] In the `pigro` library, the double call to `dependencies(nullptr)` is optimized to a single call.

```cpp
template<typename T, std::same_as<bool> B>
struct lazy_result {
    T value;
    B is_changed;
};

template<typename F>
concept lazy_function = requires(F f) {
    { f(nullptr).value };
    { f(nullptr).is_changed };
};

template<typename F>
concept lazy_function_with_facade = lazy_function<F> && std::invocable<F>;

auto lazy_value(auto value, auto is_changed) {
    return [=](std::nullptr_t) {
        return lazy_result{value, is_changed};
    };
}
```

```cpp
auto f = lazy_value(42, true); // always considered to be changed
auto g = lazy_value(1729, false); // always considered up-to-date
```

```cpp
auto ensure_lazy_function(lazy_function auto dependency) {
    return dependency;
}

auto ensure_lazy_function(lazy_function_with_facade auto dependency) {
    return dependency;
}

auto ensure_lazy_function(std::invocable auto dependency) {
    return lazy(
        [=](int) mutable { return dependency(); },
        lazy_value(0, true)
    );
}

auto ensure_lazy_function(auto dependency) {
    return lazy_value(dependency, false);
}
```

```cpp
auto facade(lazy_function auto f) {
    return [=]<typename ...Args>(Args ...args) mutable {
        if constexpr (std::same_as<std::tuple<Args...>, std::tuple<std::nullptr_t>>)
            return f(nullptr);
        else if (sizeof...(Args) == 0)
            return f(nullptr).value;
    };
}

auto lazy(auto f, auto ...dependencies) {
    return facade(lazy_core(f, ensure_lazy_function(dependencies)...));
}
```
