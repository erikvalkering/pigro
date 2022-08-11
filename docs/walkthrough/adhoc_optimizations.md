# Ad-hoc optimizations

One possible use is for example to avoid the need for a cache for the mouse position altogether if there is the availability of some system call that returns whether it was changed:

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
