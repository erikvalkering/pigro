# Extra laziness
One drawback of passing the image as a value dependency, is that it will be evaluated *immediately*, even in the case that it might never be used. For example, it the following case this would unnecessarily load resources from disk:
```cpp
auto render_mouse_cursor(const point_2d pos, const image &icon) -> ui_object;
auto get_mouse_pos() -> point_2d;
auto load_image(const std::string_view filename) -> image;

auto mouse_cursor = pigro::lazy(render_mouse_cursor, get_mouse_pos, load_image("arrow.png"));

auto is_mouse_hidden = ...;

// Rendering loop for a graphical editor
while (true) {
    if (!is_mouse_hidden) {
        mouse_cursor();
    }
}
```

This can now also be fixed quite easily, by making the arrow fully lazy:
```cpp
auto arrow = pigro::lazy(load_image, "arrow.png");
auto mouse_cursor = pigro::lazy(render_mouse_cursor, get_mouse_pos, arrow);
```

Now, the image won't even be loaded until when it is actually needed. Afterwards, the previously cached value will be used.
