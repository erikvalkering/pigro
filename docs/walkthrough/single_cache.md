# Avoiding double dependencies cache

- First of all, we are going to have all of our lazy functions return the flag indicating whether it was changed or not. 
- Second of all, we don't need to store the cache for the dependencies. If we can assume that all of our dependencies are in fact lazy functions, we know that they have this cache already inside of them.

We start by introducing a helper struct, `lazy_result`, that can hold the result of a lazy function in addition to the flag indicating whether it was changed with respect to the previous time it was called:
```cpp
template<typename T>
struct lazy_result {
    bool is_changed;
    T value;
};
```

Now, we will make some changes to our `lazy()` function to incorporate the new design. First, we'll remove the dependencies cache, which is no longer necessary as it is already being handled by the dependencies themselves. Second, we need to take into account the fact that all of the dependencies are returning a `lazy_result` when invoked, instead of _only_ the result.

The new code will be:
```cpp
auto lazy(auto f, auto ...dependencies) {
    auto cache = std::optional<decltype(f(dependencies().value...))>{};

    return [=]() mutable {
        if (!cache || (dependencies().is_changed || ...)) {
            cache = f(dependencies().value...);
        }

        return *cache;
    };
}
```

>[!note] In the above code, there is a small inefficiency: the double call to `dependencies()`. In a later section, we will discuss how this can be optimized into a single call.

- This result in a very flexible design, as it allows us to customize completely the result of the lazy function in an ad-hoc fashion (i.e. if pigro::lazy doesn't support some use case, we can just create a new function that returns a lazy_result).

Let's see how far we can get re-implementing the previous graphical editor. First, the most basic form of caching should be supported:

Now we can start defining our lazy functions using this improved design:
```cpp
auto render_mouse_cursor(const point_2d pos, const image &icon) -> ui_object;
auto get_mouse_pos() -> point_2d;
auto load_image(const std::string_view filename) -> image;

auto arrow() {
    auto cache = std::optional<image>{};
    return [=]() mutable {
        if (!cache) cache = load_image("arrow.png");
        return {false, *cache};
    };
}

auto mouse_pos() {
    auto cache = std::optional<point_2d>{};
    return [=]() mutable {
        auto pos = get_mouse_pos();
        return {
            cache != pos,
            *cache = pos,
        };
    };
}

auto mouse_cursor = pigro::lazy(render_mouse_cursor, mouse_pos, arrow);

// Rendering loop for a graphical editor
while (true) {
    mouse_cursor();
}
```

This demonstrates the basic idea of the new design: the invalidation of the dependent lazy function is triggered by the dependencies, through the `lazy_result::is_changed` flag, whereas in the previous design the dependencies cache was compared against the new values returned by the dependencies.
Apart from halving the memory footprint required for storing the cached values, this new design allows for a lot of flexibility, which opens up quite some interesting uses of the library.
One possible use is for example to avoid the need for a cache for the mouse position altogether, if there is the availability of some system call that returns whether it was changed:

```cpp
auto get_mouse_pos() -> point_2d;
auto has_mouse_pos_changed() -> bool;

auto mouse_pos() {
    return [] {
        return {
            has_mouse_pos_changed(),
            get_mouse_pos(),
        };
    };
}
```

Unfortunately, in the above examples, defining the dependencies themselves still requires quite some boilerplate. Furthermore, there is quite some overlap between those functions and the `lazy()` function. Finally, it is not possible yet to define a lazy function using the `lazy()` function, and use that as a dependency for another lazy function that is also defined using the `lazy()` function. In the next section, we'll discuss how we can further improve on the new design to simplify this even further, and tackle all of these problems.
