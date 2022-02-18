# Dependencies
In the previous two examples, the main use case for both was to ensure that the wrapped function is called at most once. However, in some cases an event might require that the wrapped function to be called again. Consider for example a graphical editor, with a rendering loop, in which we need to render a mouse cursor:

```cpp
auto render_mouse_cursor(const point_2d pos, const image &icon) -> ui_object;
auto get_mouse_pos() -> point_2d;
auto load_image(const std::string_view filename) -> image;

// Rendering loop for a graphical editor
while (true) {
    render_mouse_cursor(get_mouse_pos(), load_image("arrow.png"));
}
```

As an optimization, we might want to skip the render call if the mouse was not moved. Unfortunately, simply wrapping the render call inside of `pigro::lazy()` won't help here, because that will render the mouse cursor **only** once, instead of only when the mouse is being moved. We'd somehow need to "invalidate" the cache or the `is_called` flag in case the mouse was moved.

Fortunately, the `pigro::lazy()` utility also has this use case covered, and this is actually where the Pigro library shines: Reactive Programming.

In addition to passing the render function, `pigro::lazy()` accepts additional **dependencies** - _functions_ which are supposed to provide the inputs to the wrapped function:
```cpp
auto render_mouse_cursor(const point_2d pos, const image &icon) -> ui_object;
auto get_mouse_pos() -> point_2d;
auto load_image(const std::string_view filename) -> image;

auto arrow = [] { return load_image("arrow.png"); };
auto mouse_cursor = pigro::lazy(render_mouse_cursor, get_mouse_pos, arrow);

// Rendering loop for a graphical editor
while (true) {
    mouse_cursor();
}
```

Now, when the lazy `mouse_cursor()` function is being called, it will first check whether any dependency has changed, and if so, only then it will call the actual wrapped function. As a result, the `render_mouse_cursor()` function will _only_ be called if the mouse was in fact moved (i.e. if `get_mouse_pos()` returned a different value). A look behind the scenes might give away the magic that is going on (again **heavily simplified**):

```cpp
auto lazy(auto f, auto ...dependencies) {
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

This new version of the `pigro::lazy()` utility keeps track of two caches: one for the wrapped function, and another that bundles all of the return values from the dependencies. Now, in order to determine whether we should call our wrapped function, we simply check whether the function cache was previously filled, or whether any of the dependencies have changed, by comparing them against this second _dependencies_ cache.

Although this is slightly more complex than the previous version, it opens up nice new possibilities.

A keen eye may have noticed that there is some redundant work being done. Each time that the mouse is being moved, we render the mouse cursor. However, we are also loading the arrow image again and again, even though this image might not change.

With the current functionality, this issue is easily fixed:
```cpp
// ...
auto arrow = pigro::lazy([] { return load_image("arrow.png"); });
auto mouse_cursor = pigro::lazy(render_mouse_cursor, get_mouse_pos, arrow);
// ...
```

Now, the `arrow()` function can be used as dependency for the `render_mouse_cursor()` function, while at the same time being optimized to be called only once.
