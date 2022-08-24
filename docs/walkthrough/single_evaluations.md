# Single evaluations
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
