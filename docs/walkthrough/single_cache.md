# Avoiding double dependencies cache

- First of all, we are going to have all of our lazy functions return the flag indicating whether it was changed or not.
- Second of all, we don't need to store the cache for the dependencies. If we can assume that all of our dependencies are in fact lazy functions, we know that they have this cache already inside of them.

We start by introducing a helper struct, `lazy_result`, that can hold the result of a lazy function in addition to the flag indicating whether it was changed with respect to the previous time it was called:
```cpp
template<typename T>
struct lazy_result {
    bool is_changed;
    T result;
};
```

Now, we will make some changes to our `lazy()` function to incorporate the new design. First, we'll remove the dependencies cache, which is no longer necessary as it is already being handled by the dependencies themselves that are captured inside of lazy function. Second, we need to take into account the fact that all of the dependencies are returning a `lazy_result` when invoked, instead of _only_ the result. Finally, we need to make the lazy function return a lazy_result as well, such that it can be used as a dependency itself.

The new code will be:
```cpp
auto lazy(auto f, std::invocable auto ...dependencies) {
    auto cache = std::optional<decltype(f(dependencies().result...))>{};

    return [=]() mutable {
        auto is_changed = (dependencies().is_changed || ... || !cache);
        if (is_changed) {
            const auto result = f(dependencies().result...);
            is_changed = result != cache;
            cache = result;
        }

        return lazy_result{is_changed, *cache};
    };
}
```

// TODO: discuss this:
- removed dependencies cache
- result from .result member
- return lazy_result
- is_changed check replaces (unconditional) dependencies_cache comparison
- extra comparison if reevaluated
// TODO: last comparison could be made configurable

// TODO: Integrate this paragraph
Because the dependencies are now properly constrained to be `lazy` dependencies, we can invoke them using the `nullptr` argument and first check the `is_changed` flag to see whether they in fact have changed, before calculating the new result. After having calculated the new result, we should still check whether the value has actually changed (which was done in the previous design, but unconditionally). Finally, we store the result in the cache, and return it together with the flag (both combined as a `lazy_result`).
// TODO: END - Integrate this paragraph

>[!note] In the above code, there is a small inefficiency: the double call to `dependencies()`. In [a later section](walkthrough/single_evaluations.md), we will discuss how this can be optimized into a single call.

- This result in a very flexible design, as it allows us to customize completely the result of the lazy function in an ad-hoc fashion (i.e. if pigro::lazy doesn't support some use case, we can just create a new function that returns a lazy_result).

Now we can start defining our lazy functions using this improved design.

To adapt the dependencies into a form that is accepted by the pigro::lazy() function (i.e. when invoking them, they should return a `lazy_result`), we have to create a couple of helper functions.

We start with defining the `arrow` lazy function, which is used as the `icon` dependency to the `mouse_cursor` lazy function. We want that once this function is called, its return value is cached. Fortunately, this is supported out of the box:
```cpp
auto arrow = pigro::lazy([] { return load_image("arrow.png"); });
```

In this case, the lazy function that is created has zero dependencies. As a result, the `load_image()` function is invoked only once and the result is cached and returned. The next time the lazy function is called, the cache is not empty anymore, so the value inside it is returned immediately.

For the `pos` dependency of the `mouse_cursor`, it's a bit more involved. This is because we want the `get_mouse_pos()` function to be always called, but we should compare it with the previously cached value, such that we can determine whether it changed.

Because all that is required from a dependency in the new design, is for it to return a `lazy_result`, we can implement one in an ad-hoc fashion:
```cpp
auto mouse_pos() {
    auto cache = std::optional<point_2d>{};

    return [=]() mutable {
        const auto result = get_mouse_pos();
        const auto is_changed = result != cache;
        cache = result;

        return lazy_result{is_changed, *cache};
    };
)
```

This also demonstrates the flexibility of the new design: if some specific use case is not supported (yet) by the library, we can easily hand-code a solution that satisfies the requirements. Because the requirements of the dependencies are really minimal, we have a lot of freedom in the solution.

However, there is a bit of code duplication between this ad-hoc implementation and the main `pigro::lazy()` function. Let's see whether we can be a bit creative and reuse the `pigro::lazy()` function.

We start by defining a little helper dependency, named `always_changed()`, whose sole purpose is to always invalidate the lazy function that depends on it:
```cpp
auto always_changed = [] {
    return lazy_result{ true, 0 };
};
```

As can be seen, the `always_changed()` helper function returns a `lazy_result`, and can therefore be used as a dependency. The actual result of the value, `0`, is just a dummy value and can be ignored by the depending function.

Now, we can define the actual `mouse_pos` lazy function:
```cpp
auto mouse_pos = pigro::lazy(
    [](int) { return get_mouse_pos(); },
    always_changed
);
```

Because we are providing an additional dependency, `always_changed`, we need to wrap the `get_mouse_pos` function with something that "swallows" the result of that dependency. Now, whenever this dependency is called, it will always invoke `get_mouse_pos`, because the `always_changed` dependency indicates that it has changed. However, it will also compare the value with the previously cached result, such that the dependent lazy function (i.e. `mouse_cursor`) will not be invoked.

Looking closer to what this expands into, will make it very clear what is going on.
First, we'll substitute `dependencies` (`always_changed`, a.k.a. `[] { return lazy_result{ true, 0 }; }`):
```cpp
auto lazy(auto f, auto ...dependencies) {
    auto cache = std::optional<decltype(f(always_changed().result))>{};

    return [=]() mutable {
        auto is_changed = always_changed().is_changed || !cache;
        if (is_changed) {
            const auto result = f(always_changed().result);
            is_changed = result != cache;
            cache = result;
        }

        return lazy_result{is_changed, *cache};
    };
}
```

Now, we'll substitute `f` (`[](int) { return get_mouse_pos(); }`):
```cpp
auto lazy(auto f, auto ...dependencies) {
    auto cache = std::optional<point_2d>{};

    return [=]() mutable {
        auto is_changed = true || !cache;
        if (is_changed) {
            const auto result = get_mouse_pos();
            is_changed = result != cache;
            cache = result;
        }

        return lazy_result{is_changed, *cache};
    };
}
```

Now, considering that `is_changed` is always `true`, it is very likely that the compiler will optimize this into:
```cpp
auto lazy(auto f, auto ...dependencies) {
    auto cache = std::optional<point_2d>{};

    return [=]() mutable {
        const auto result = get_mouse_pos();
        auto is_changed = result != cache;
        cache = result;

        return lazy_result{is_changed, *cache};
    };
}
```

As can be seen, the final listing is identical to the ad-hoc implementation we started with. On the other hand, it required us a lot less typing to achieve that.

Finally, we can all tie it together, and define the actual `mouse_cursor` lazy function. The implementation is as simple as in the previous design:
```cpp
auto mouse_cursor = pigro::lazy(render_mouse_cursor, mouse_pos, arrow);
```

This demonstrates the basic idea of the new design: the invalidation of each lazy function is triggered by its dependencies, through the `lazy_result::is_changed` flag, whereas in the previous design the dependencies cache was compared against the new values returned by the dependencies. This has a very important impact on the performance, as it allows us to avoid potentially expensive comparisons.

Secondly, we have now almost halved the memory footprint required for storing the cached values. Finally, as will be seen in later sections, this new design allows for a lot of flexibility, which opens up quite some interesting optimizations for the library.

To give a complete picture of how we can implement the graphics editor example using the new design, here is the full listing:
```cpp
auto render_mouse_cursor(const point_2d pos, const image &icon) -> ui_object;
auto get_mouse_pos() -> point_2d;
auto load_image(const std::string_view filename) -> image;

auto arrow = pigro::lazy([] { return load_image("arrow.png"); });

auto always_changed() {
    return lazy_result{ true, 0 };
}

auto mouse_pos = pigro::lazy(
    [](int) { return get_mouse_pos(); },
    always_changed
);

auto mouse_cursor = pigro::lazy(render_mouse_cursor, mouse_pos, arrow);

// Rendering loop for a graphical editor
while (true) {
    mouse_cursor();
}
```

Although the above is already quite concise, if we compare it to the previous design, we can see that for the mouse_pos, we needed to manually introduce an additional lazy function. In the previous design, this was not necessary, as the pigro::lazy() function would already support normal functions out of the box (in fact, that was the _only_ requirement).

In the next section, we will discuss how we can further improve on that, and make it as simple as before.

// TODO: Integrate this paragraph
One big difference with the previous design, apart from the algorithmic improvement (i.e. avoiding comparisons), is that we reduced the memory footprint considerably: each lazy function now needs to store only the cache for its own calculated result, whereas previously, it would also hold the caches for all of its dependencies (even though they were already stored in the dependencies themselves if they were lazy). On top of that, in the previous design, for lazy functions having zero dependencies, we would still have to store an `std::optional<std::tuple<>>`, whereas in the new design it no longer necessary.

However, the underlying implementation is much more efficient, both from a work as well as memory perspective.
// TODO: END - Integrate this paragraph

