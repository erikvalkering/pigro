# Avoiding comparisons
Consider again a somewhat simpler version of the previous code fragment, in which we made the arrow icon also lazy:
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

There is a subtle performance issue here: if we don't move the mouse, we don't need to redraw the mouse cursor. You'd therefore expect the call to `mouse_cursor()` to be a very cheap operation. Unfortunately, with the current design of the `pigro::lazy()` function, we have to compare *all* of the evaluations of the dependencies with those previously cached, including the `arrow` dependency. Even though the *evaluation* of the `arrow` dependency is very cheap (i.e. it returns the cached value instead of reloading the image), the comparison is not cheap at all: it needs to perform a comparison between two `image` objects each time before `mouse_cursor()` realizes nothing needs to be done.
