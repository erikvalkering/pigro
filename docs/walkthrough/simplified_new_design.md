# Simplified new design

```cpp
auto lazy(auto f, auto ...dependencies) {
    auto cache = std::optional<decltype(f(dependencies().value...))>{};

    return [=]() mutable {
        auto is_changed = !cache || (dependencies().is_changed || ...);
        if (is_changed) {
            auto result = f(dependencies().value...);
            is_changed = result != cache;
            cache = result;
        }

        return {is_changed, *cache};
    };
}
```

With this small change, i.e. the lazy functions returned by the `lazy()` function, now also return a `lazy_result`, we can suddenly use lazy functions defined by the `lazy()` function as dependenciess of new lazy functions defined by `lazy()`.

The previous example can now be simplified to this:
```cpp
auto render_mouse_cursor(const point_2d pos, const image &icon) -> ui_object;
auto get_mouse_pos() -> point_2d;
auto load_image(const std::string_view filename) -> image;

auto image_filename() {
    return lazy_result{false, "arrow.png"};
}

auto arrow = pigro::lazy(load_image, image_filename);
auto mouse_pos = pigro::lazy(
    [](int) { return get_mouse_pos(); },
    [] { return lazy_result{ true, 0 }; }
);

auto mouse_cursor = pigro::lazy(render_mouse_cursor, mouse_pos, arrow);

// Rendering loop for a graphical editor
while (true) {
    mouse_cursor();
}

// TODO: facade
// TODO: simplify further dependencies using ensure_lazy/dep
```
