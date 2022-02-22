# Final design

Here follows an overview of the final design of the library, in less than 70 lines of code:

```cpp
namespace pigro {
namespace concepts {

template<typename F>
concept lazy = requires(F f) {
    { f(nullptr).value };
    { f(nullptr).is_changed };
};

} // namespace concepts

template<typename T, std::same_as<bool> B>
struct lazy_result {
    T value;
    B is_changed;
};

auto lazy_core(auto f, concepts::lazy auto ...dependencies) {
    auto cache = std::optional<decltype(f(dependencies(nullptr).value...))>{};

    return [=](std::nullptr_t) mutable {
        auto changed = !cache || (dependencies(nullptr).is_changed || ...);
        if (changed) {
            auto result = f(dependencies(nullptr).value...);
            changed = result != cache;
            cache = result;
        }

        return lazy_result{*cache, changed};
    };
}

auto ensure_lazy(concepts::lazy auto dependency) {
    return dependency;
}

auto ensure_lazy(std::invocable auto dependency) requires concepts::lazy<decltype(dependency)> {
    return dependency;
}

auto ensure_lazy(auto dependency) {
    return lazy_value(dependency, false);
}

auto ensure_lazy(std::invocable auto dependency) {
    return lazy(
        [=](int) mutable { return dependency(); },
        lazy_value(0, true)
    );
}

auto facade(concepts::lazy auto f) {
    return [=](auto ...args) mutable {
        if constexpr (sizeof...(args) == 0)
            return f(nullptr).value;
        else
            return f(nullptr);
    };
}

auto lazy(auto f, auto ...dependencies) {
    return facade(lazy_core(f, ensure_lazy(dependencies)...));
}

} // namespace pigro
```
