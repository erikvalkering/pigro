# Single evaluations
As was already mentioned before, the new design has an important performance regression that we promised to fix.

If we look again at our example:
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

Each time we invoke `mouse_cursor()`, all of the dependencies are evaluated twice: once to determine whether they have changed, by querying the `is_changed` data member of the `lazy_result` returned by each dependency, and another time to pass the actual `result` to the wrapped function, in case one of the dependencies did change.

Looking at the implementation of the `lazy` function makes clear why that is:
```cpp
auto lazy(auto f, lazy_dependency auto ...dependencies) {
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

Even though each dependency is evaluated twice, this doesn't necessary mean that the wrapped function is in fact evaluated twice. If the dependency is also a lazy function, then it will use the machinery of the `pigro::lazy()` function, and therefore only re-evaluate the wrapped function if its dependencies have changed. Otherwise, if the dependency was a value (wrapped using one of the `ensure_lazy_dependency()` overloads), it will not be a problem anyway, as it's a very cheap operation (i.e. it will simply return a previously-cached value, flagging it as "unchanged").
However, if the dependency was a normal function (also wrapped using one of the `ensure_lazy_dependency()` overloads), it will unconditionally re-evaluate the function, as well as compare it to the previously-cached value. Both of these operations may be costly, so doing them twice is obviously very inefficient.

Furthermore, apart from the performance issue, it may potentially also result in some unexpected behaviour, if a dependency returns different values between two subsequent invocations. Consider for example a custom non-lazy "on-mouse-click" event that can be used as a dependency:
```cpp
auto get_mouse_button_state() -> button_state;

auto on_mouse_click() {
    return [state = button_state::up]() mutable {
        const auto new_state = get_mouse_button_state();
        const auto was_clicked = state != new_state
                              && new_state == button_state::up,
        state = new_state;

        return lazy_result{
            was_clicked,
            was_clicked,
        };
    };
}
```
This dependency keeps track of transitions in the mouse button's up/down state and will only flag itself as changed in case it detects a transition and if the button state is up, i.e the button was clicked.
// TODO: add example for manually optimizing away the cache
```cpp
auto mouse_pos = [] {
    return lazy_result{
        true,
        get_mouse_pos(),
    };
};
```
// This is more efficient, if the likelyhood of returning equal mouse cursor coordinates
// is low.

It can be used for example to implement a click-counter:
```cpp
auto counter = pigro::lazy(
    [count = 0](auto on_mouse_click) mutable {
        if (on_mouse_click) {
            cout << "Count: " << ++count << endl;
        }
    },
    on_mouse_click
);

while (true) {
    counter();
}
```

```cpp
auto lazy(auto f, lazy_dependency auto ...dependencies) {
    auto cache = std::optional<decltype(f(dependencies().result...))>{};

    return facade{[=]() mutable {
        return [&, ...args = dependencies()] {
            auto is_changed = (args.is_changed || ... || !cache);
            if (is_changed) {
                const auto result = f(args.result...);
                is_changed = result != cache;
                cache = result;
            }

            return lazy_result{is_changed, *cache};
        }()
    }};
}
```
