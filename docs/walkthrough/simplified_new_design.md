# Syntactic sugar

Using the new design, we efficiently support all of the use cases, though with a small bit of boilerplate required for dependencies that are not yet _lazy_.

We can use the same approach, that we took in the [Syntactic sugar](walkthrough/syntactic_sugar.md) section, to simplify this. Though this time, we don't just want to constrain the main `pigro::lazy()` function using `std::invocable`, which would merely ensure that all of the dependencies are callable. We want to actually make sure that when calling them, they return a `lazy_result`. This way, we can add an unconstrained `pigro::lazy()` overload that adapts all of the dependencies that do not yet satisfy this requirement.

We'll start by introducing a new concept, called `lazy_dependency`:
```cpp
template<typename F>
concept lazy_dependency = std::invocable<F> && requires(F f) {
    f().is_changed;
    f().result;
};
```

This concept will satisfy any object that is `std::invocable` using zero parameters, and for which the result contains at least an `is_changed` and `result` data member.

Now, we can constrain the main `lazy()` overload using this concept (the actual implementation remains unchanged):
```cpp
auto lazy(auto f, lazy_dependency auto ...dependencies) {
    // ...as before...
}
```

This main `lazy()` overload will be selected only if _all_ of the dependencies satisfy the `lazy_dependency` concept (i.e. they can be invoked _and_ return a `lazy_result`).

Now, we can define an _unconstrained_ overload for `pigro::lazy()`:
```cpp
auto lazy(auto f, auto ...dependencies) {
    return lazy(f, ensure_lazy_dependency(dependencies)...);
}
```

This overload will be selected if not all of the dependencies satisfy the `lazy_dependency` concept. The only additional thing that it will do, is adapt each dependency such that it will satisfy the concept. Subsequently, it can now forward the call to the main template, since all the dependencies are satisfying the concept.

In order to adapt each dependency, we introduce an overload set called `ensure_lazy_dependency()`.

If the dependency is already lazy, there is nothing to do, so we can return it as-is:
```cpp
auto ensure_lazy_dependency(lazy_dependency auto f) {
    return f;
}
```

Instead, if it is a normal function (i.e. a `std::invocable`), we need to adapt it, similar to what we did for the `mouse_pos()` lazy function in the previous section:
```cpp
auto ensure_lazy_dependency(std::invocable auto f) {
    return lazy(
        [=](int) { return f(); },
        always_changed
    );
}
```

Otherwise, we assume it is a value. In this case, the implementation is really simple: just return a `lazy_result` containing the value, and flags it as "never changed":
```cpp
auto ensure_lazy_dependency(auto value) {
    return [=] {
        return lazy_result{false, value};
    };
}
```

The previous example can now be simplified to this:
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
