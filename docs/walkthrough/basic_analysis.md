# Analysis
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

There is still a subtle performance issue here: if we don't move the mouse, we don't need to redraw the mouse cursor. You'd therefore expect the call to `mouse_cursor()` to be a very cheap operation. Unfortunately, as it will turn out, this operation is far from cheap. Before the `mouse_cursor()` lazy function realizes nothing needs to be done, it needs to check whether any of its dependencies have changed.

For the first dependency, we are simply calling the non-lazy `get_mouse_pos()` function and compare it against the previously cached value, which turns out to be the same. This comparison cannot be avoided, but is fortunately rather cheap, just a comparison of two points.

Next, we check the `arrow` dependency. Even though the *evaluation* is very cheap (i.e. being a lazy function dependency, it returns the cached value instead of reloading the image), the *comparison* may not be cheap at all: it needs to perform a comparison between two `image` objects in order to determine whether the dependency has changed. Depending on how the `image` type is implemented, this may be a very costly operation, worst-case involving a comparison of all the pixel data.

In order to get a better understanding of the issue in general, let's revisit the current design that we have of the `pigro::lazy()` function:
```cpp
auto lazy(auto f, std::invocable auto ...dependencies) {
    auto cache = std::optional<decltype(f(dependencies()...))>{};
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

As can be seen, in order to determine whether we need to reevaluate the lazy function, we are **unconditionally** comparing *all* of the evaluations of the dependencies with those previously cached (i.e. the `args != dependencies_cache` bit), even if they did **not** change.

What we need, is a way to tell whether the value returned from the dependency was newly computed, or not, in which case we can avoid the comparison. There are three different cases to consider here:
```cpp
auto foo(int bar) -> int;
auto bar() -> int;

auto foo_a = pigro::lazy(foo, bar);
auto foo_b = pigro::lazy(foo, foo_a);
auto foo_c = pigro::lazy(foo, 42);
```

If the dependency was a simple function (case `foo_a`, which has `bar` as the dependency, i.e. a normal *non-lazy* function), it will always be recomputed and we are required to compare against the previously computed value. If the dependency is a lazy function (case `foo_b`, which has `foo_a` as a the dependency, i.e. a lazy function), we could in principle avoid the comparison if the dependency wasn't recomputed (i.e. `foo_a` didn't change, which means in turn that `bar` didn't change). Finally, if the dependency is a value (case `foo_c`, which has the *constant* value `42` as the dependency), we also know that we don't need to compare the value, as it can't possibly change. Therefore, we should only consider this latter case: dependencies that are lazy (i.e. case `foo_b`).

