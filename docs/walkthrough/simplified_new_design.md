# Simplified new design

Using the new design, we efficiently support all of the use cases, though with a small bit of boilerplate required for dependencies that are not yet _lazy_.

We can use the same approach to simplify this, that we took in the [Syntactic sugar](walkthrough/syntactic_sugar.md) section. Though this time, we don't just want to constrain the main `pigro::lazy()` function using `std::invocable`, which would merely ensure that all of the dependencies are callable. We want to actually make sure that when calling them, they return a `lazy_result`. This way, we can add an unconstrained `pigro::lazy()` overload that adapts all of the dependencies that do not yet satisfy this requirement.

We'll start by introducing a new concept, called `lazy_dependency`:
```cpp
template<std::invocable F>
concept lazy_dependency = requires(F f) {
    f().is_changed;
    f().result;
};
```

This concept will satisfy any object that is `std::invocable` using zero parameters, and for which the result contains at least an `is_changed` and `result` data member.

Now, we can constrain the main template using this concept:
```cpp
auto lazy(auto f, lazy_dependency auto ...dependencies) {
    // ...as before...
}
```

And define the unconstrained overload:
```cpp
auto lazy(auto f, auto ...dependencies) {
    return lazy(f, ensure_lazy_dependency(dependencies)...);
}
```

```cpp
auto ensure_lazy_dependency(lazy_dependency auto f) {
    return f;
}

auto ensure_lazy_dependency(std::invocable auto f) {
    return lazy(
        [=](int) { return f(); },
        always_changed
    );
}
```

```cpp
auto ensure_lazy_dependency(auto constant) {
    return [=] {
        return lazy_result{false, constant};
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
```
