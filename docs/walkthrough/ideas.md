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
