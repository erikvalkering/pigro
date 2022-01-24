# Syntactic sugar
Because the previous pattern occurs quite often, i.e. having a constant-valued dependency that should be cached, there is a short-hand syntax available such that we can pass values directly as dependencies to the `pigro::lazy()` utility:
```cpp
auto mouse_cursor = pigro::lazy(render_mouse_cursor, get_mouse_pos, load_image("arrow.png"));
```

This will wrap the image dependency with a lazy function, such that the value will be cached and used inside of the `render_mouse_cursor()` function, instead of loading the images every time.

In order to make this work, we actually don't need to modify the implementation of the existing `pigro::lazy()` utility, but merely constrain it a bit and add a new overload:
```cpp
auto lazy(auto f, std::invocable auto ...dependencies) {
    // ...as before...
}

auto ensure_invocable(std::invocable auto dependency) {
     return dependency;
}

auto ensure_invocable(auto dependency) {
    return lazy([=] { return dependency; });
}

auto lazy(auto f, auto ...dependencies) {
    return lazy(f, ensure_invocable(dependencies)...);
}
```

The first `lazy`-overload is the previous `lazy()` implementation, but now constrained using `std::invocable`, such that it will only be selected if all of the dependencies can be called. The second `lazy`-overload will be selected otherwise, and simply transforms each dependency into a form that can be invoked as a function, and then delegates to the first overload. The two `ensure_invocable()` overloads are helpers that perform this transformation. The first will be selected for dependencies that are already callable, and therefore returns them as-is. The second one creates a function that returns the dependency and subsequently wraps it with `lazy()` to make sure that it will be called only once (and therefore the cache will be used).
