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

Looking at the implementation of the `lazy` function, we can see why that is:
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
However, if the dependency was a normal function (also wrapped using one of the `ensure_lazy_dependency()` overloads), it will unconditionally re-evaluate the function, as well as compare it to the previously-cached value. Both of these operations may be costly, so doing them twice is a potential performance bottleneck.

Furthermore, apart from the performance issue, it may potentially also result in some unexpected behavior, if a dependency returns different values between two subsequent invocations. Consider for example a custom non-lazy "on-mouse-click" event that can be used as a dependency:
```cpp
auto get_mouse_button_state() -> button_state;

auto on_mouse_click() {
    return [state = button_state::up]() mutable {
        const auto new_state = get_mouse_button_state();
        const auto was_clicked = state != new_state
                              && new_state == button_state::up,
        state = new_state;

        return lazy_result{
            .is_changed: was_clicked,
            .value: was_clicked,
        };
    };
}
```
This dependency keeps track of transitions in the mouse button's up/down state and will only flag itself as changed in case it detects a transition from down to up, i.e the button was clicked.

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

This will not give the correct result: when the `on_mouse_click()` dependency of the `counter()` lazy-function is being checked and the mouse was clicked, it will execute the function, because the `is_changed` portion of the `lazy_result` object returned by `on_mouse_click()` dependency was `true`. However, because it re-evaluates the `on_mouse_click()` function again in order to determine the value of the `value` portion, that value will always be `false`, because the `new_state` will always equal the previous `state`. As a result, the count will never be printed. Therefore, this issue is not only a matter of performance, but also a matter of correctness.

There are several ways how this can be solved. One way would be to capture the `lazy_result` objects in a tuple and subsequently access the `is_changed` and `value` portions from that tuple:
```cpp
auto lazy(auto f, lazy_dependency auto ...dependencies) {
    auto cache = std::optional<decltype(f(dependencies().result...))>{};

    return facade{[=]() mutable {
        const auto args = std::tuple{dependencies()...};
        auto is_changed = any(args, is_changed) || !cache;
        if (is_changed) {
            const auto results = transform(args, get_result);
            const auto result = std::apply(f, results);

            is_changed = result != cache;
            cache = result;
        }

        return lazy_result{is_changed, *cache};
    }};
}
```

Where:
```cpp
inline constexpr auto get_result = [](const concepts::lazy_result auto result) {
    return result.value;
};

inline constexpr auto is_changed = [](const concepts::lazy_result auto result) {
    return result.is_changed;
};
```

and
```cpp
constexpr auto transform(const auto &data, auto projection) {
    return std::apply([=](const auto &...args) {
        return std::tuple{ projection(args)... };
    },
      data);
}

template<typename Predicate = std::identity>
constexpr auto any(const auto &data, Predicate predicate = {}) {
    return std::apply([=](const auto &...args) {
        return (predicate(args) || ...);
    },
      data);
}
```

In an earlier iteration of this library, this approach was used. However, while working on the presentation, and realizing this would require quite some extra time to explain, I decided to leave the double-evaluation inefficiency, with a remark that this was solved in the real library. Later, I found a much more elegant way to do this, without relying on capturing the `lazy_result` objects in a tuple, and using the tuple algorithms to access them. Instead, I was using yet another lambda trick:
```cpp
auto lazy(auto f, lazy_dependency auto ...dependencies) {
    auto cache = std::optional<decltype(f(dependencies().result...))>{};

    return facade{[=]() mutable {
        return [&](auto ...args) {
            auto is_changed = (args.is_changed || ... || !cache);
            if (is_changed) {
                const auto result = f(args.result...);
                is_changed = result != cache;
                cache = result;
            }

            return lazy_result{is_changed, *cache};
        }(...dependencies());
    }};
}
```

So here we wrap the code inside of a lambda, and evaluate all the dependencies only once, while passing the `lazy_result` objects as a variadic pack to the lambda, which can then access them directly as regular objects, instead of indirectly using one of the ... functions.

In C++20, this can be made even a bit shorter, by using a variadic pack capture:
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
        }();
    }};
}
```

Interestingly, as we will see later, all of these different approaches have a significant effect on the generated code. In the benchmark section, we will go into the differences between them.

> In C++26, we might get something which could even remove the need for a wrapping lambda, such that the code could be simplified even further:
```cpp
auto lazy(auto f, lazy_dependency auto ...dependencies) {
    auto cache = std::optional<decltype(f(dependencies().result...))>{};

    return facade{[=]() mutable {
        const auto ...args = dependencies();
        auto is_changed = (args.is_changed || ... || !cache);
        if (is_changed) {
            const auto result = f(args.result...);
            is_changed = result != cache;
            cache = result;
        }

        return lazy_result{is_changed, *cache};
    }};
}
```
