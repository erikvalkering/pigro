## Reactivity all the way down
```c++
auto render_mouse_cursor(const point_2d pos, const image &icon) -> ui_object;
auto get_mouse_pos() -> point_2d;
auto load_image(const std::string_view filename) -> image;
auto get_drawing_mode() -> drawing_mode;

auto get_mouse_icon_filename(const drawing_mode mode) {
    return mode == drawing_mode::drawing ? "crosshair.png" : "arrow.png";
}

auto mode = lazy(get_drawing_mode);
auto filename = lazy(get_mouse_icon_filename, mode);
auto icon = lazy(load_image, filename);
auto mouse_cursor = lazy(render_mouse_cursor, get_mouse_pos, icon);

// Rendering loop for a graphical editor
while (true) {
    mouse_cursor();
}
```
