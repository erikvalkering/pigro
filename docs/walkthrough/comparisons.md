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

There is a subtle performance issue here: if we don't move the mouse, we don't need to redraw the mouse cursor. You'd therefore expect the call to `mouse_cursor()` to be a very cheap operation. Unfortunately, with the current design of the `pigro::lazy()` function, this is far from cheap. Here is the core algorithm of the current design:
```cpp
auto lazy(auto f, std::invocable auto ...dependencies) {
    auto cache = std::optional<decltype(f())>{};
    auto dependencies_cache = std::optional<decltype(std::tuple{dependencies()})>{};

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

As can be seen, in order to determine whether we need to reevaluate the lazy function, we are **unconditionally** comparing *all* of the evaluations of the dependencies with those previously cached (i.e. the `args != dependencies_cache` bit). Even though the *evaluation* of the `arrow` dependency is very cheap (i.e. it returns the cached value instead of reloading the image), the comparison is not cheap at all: it needs to perform a comparison between two `image` objects each time before `mouse_cursor()` realizes nothing needs to be done.

What we need, is a way to tell whether the value returned from the dependency was newly computed, or not, in which case we don't need to perform the comparison. If the dependency was a simple function, it will always be recomputed. Only if the dependency is a lazy function (or a value, which are converted into lazy functions), the computed value will be cached within the lazy function. Therefore, we should only consider this latter case: dependencies that are lazy.
