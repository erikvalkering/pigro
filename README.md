# pigro
_Lazy evaluation on steroids_

Pigro is a C++20 library that allows you to define functions in a declarative and reactive way, resulting in code that is easier to reason about, easier to maintain, and less prone to errors.

# Lazy functions
Let's start out with the simplest use case: we want to make sure that some function is called only once. Imagine we have several functions that use some global resources and a single `initialize()` function that initializes those resources. Before calling those functions, we must make sure that the `initialize()` function has been called. Furthermore, this should be done at most once. A typical solution is to keep track of whether the `initialize()` function was called already, using a boolean flag:

```c++
auto initialize() -> void;
auto is_initialized = false;

// ...at some point
if (foo) {
    if (!is_initialized) {
        initialize();
        is_initialized = true;
    }

    use_some_global_resource();
}

// ...elsewhere
if (bar) {
    if (!is_initialized) {
        initialize();
        is_initialized = true;
    }

    use_another_global_resource();
}

// etc.
```

Apart from the boilerplate, this ties an **implicit** relationship between the `initialize()` function and the `is_initialized` flag. This would become even more of a problem if you would have several functions that need to be called at most once. In such cases, you'd need to keep track of a separate flag for each function. This in turn will have a negative impact on the maintainability of the code.

Instead, it would be better to combine the function and the flag into a single entity. This is where the `pigro::lazy()` function comes in. Using it will wrap an existing function, and make sure that any subsequent call will be ignored:
```c++
auto ensure_initialized = pigro::lazy(initialize);

// ...at some point
if (foo) {
    ensure_initialized();
    use_some_global_resource();
}

// ...elsewhere
if (bar) {
    ensure_initialized();
    use_another_global_resource();
}

// etc.
```
> Ignore the fact that there is still an implicit ordering dependency between the calls to `use_***_resource()` and `ensure_initialized()`, which is just bad design but which is not the point of this example.

The resulting code is much cleaner: there is less boilerplate, but more importantly, the `is_initialized` flag is now maintained inside of the `ensure_initialized()` function.

In order to better understand what is going on behind the scenes, the following is a **heavily simplified** version of the `pigro::lazy()` function:
```c++
auto lazy(auto f) {
    auto is_called = false;
    return [=]() mutable {
        if (is_called) return;
        
        f();
        is_called = true;
    };
}
```

So as was expected, `lazy()` simply wraps `f()` with an additional layer which keeps track of the flag and only calls `f()` when this flag is `false` and subsequently sets the flag to `true`.

# Caching
In the previous example we wrapped a function that did not return any value. Often, however, functions do return something useful. Consider for example a function that performs a relatively expensive calculation but also returns the result from that calculation. In order to support this use case, the `pigro::lazy()` utility will cache any value returned by the wrapped function. Any subsequent time that the function is called, it simply returns the previously-cached value:

```c++
auto deep_thought() {
    // simulate some very expensive calculation
    this_thread::sleep(7500000y);
    
    return 42;
}

auto lazy_deep_thought = pigro::lazy(deep_thought);

auto answer_to_life = lazy_deep_thought(); // may take a while...
assert(answer_to_life == 42);

auto universe_and_everything = lazy_deep_thought(); // instantaneous!
assert(universe_and_everything == 42);
```

As before, a **heavily simplified** version of the `pigro::lazy()` function might give a better understand of what is going on behind the scenes:
```c++
auto lazy(auto f) {
    auto cache = std::optional<decltype(f())>{};

    return [=]() mutable {
        if (cache) return *cache;
        
        cache = f();
        return *cache;
    };
}
```

# Dependencies
In the previous two examples, the main use case was to ensure that the function is called at most once. However, there may be cases in which you'd want to somehow "invalidate" the cache or the `is_called` flag.

```c++
auto draw_mouse_cursor(const point_2d pos, const image &icon) -> ui_object;
auto get_mouse_pos() -> point_2d;
auto load_image(const std::string_view filename) -> image;

// Rendering loop for a graphics editor
while (true) {
    draw_mouse_cursor(get_mouse_pos(), load_image("arrow.png"));
}
```

```
// Rendering loop for a graphics editor

while (true) {

    mouse_cursor();

}
```

```c++
auto draw_mouse_cursor(const point_2d pos, const image &icon) -> ui_object;
auto get_mouse_pos() -> point_2d;
auto load_image(const std::string_view filename) -> image;

auto arrow = [] { return load_image("arrow.png"); };
auto mouse_cursor = lazy(draw_mouse_cursor, get_mouse_pos, arrow);

// Rendering loop for a graphics editor
while (true) {
    mouse_cursor();
}
```

As a short-hand, we can also pass values directly as dependencies to the lazy function:
```c++
auto mouse_cursor = lazy(draw_mouse_cursor, get_mouse_pos, load_image("arrow.png"));

// Rendering loop for a graphics editor
while (true) {
    mouse_cursor();
}
```

In addition to being shorter, this is actually also more efficient. This is because `load_image()` is loaded only once, whereas previously it would be continuously called.


## Reactivity all the way down
```c++
auto draw_mouse_cursor(const point_2d pos, const image &icon) -> ui_object;
auto get_mouse_pos() -> point_2d;
auto load_image(const std::string_view filename) -> image;
auto get_drawing_mode() -> drawing_mode;

auto get_mouse_icon_filename(const drawing_mode mode) {
    return mode == drawing_mode::drawing ? "crosshair.png" : "arrow.png";
}

auto mode = lazy(get_drawing_mode);
auto filename = lazy(get_mouse_icon_filename, mode);
auto icon = lazy(load_image, filename);
auto mouse_cursor = lazy(draw_mouse_cursor, get_mouse_pos, icon);

// Rendering loop for a graphics editor
while (true) {
    mouse_cursor();
}
```

## Comparison with hand-coded solution
**Lazy & Reactive - using `pigro`**
```c++
auto mode = lazy(get_drawing_mode);
auto filename = lazy(get_mouse_icon_filename, mode);
auto icon = lazy(load_image, filename);
auto mouse_cursor = lazy(draw_mouse_cursor, get_mouse_pos, icon);

// Rendering loop for a graphics editor
while (true) {
    mouse_cursor();
}
```

**Hand-coded**
```c++
auto should_draw_mouse_cursor = false;
auto cache_pos = std::optional<point_2d>{};
auto cache_icon = std::optional<image>{};
auto cache_filename = std::optional<std::string>{};
auto cache_mode = std::optional<drawing_mode>{};

// Rendering loop for a graphics editor
while (true) {
    const auto pos = get_mouse_pos();
    if (cache_pos != pos) {
        cache_pos = pos;
        should_draw_mouse_cursor = true;
    }

    const auto mode = get_drawing_mode();
    if (cache_mode != mode) {
        cache_mode = mode;

        const auto filename = get_mouse_icon_filename(mode);
        if (cache_filename != filename) {
            cache_filename = filename;

            const auto icon = load_image(filename);
            if (cache_icon != icon) {
                cache_icon = icon;

                should_draw_mouse_cursor = true;
            }
        }
    }

    if (should_draw_mouse_cursor /* && cache_pos && cache_icon */) {
        draw_mouse_cursor(*cache_pos, *cache_icon);
    }
}
```

# Features
- [x] easy creation of cached functions 
- [x] reactivity by specifying dependencies
- [x] support for values, functions, and lazy functions as dependencies
- [x] skipping re-evaluation if dependencies evaluate to same values as previous ones
- [x] skipping comparison of lazy function dependencies if they were not re-evaluated
- [x] memory optimization for void functions
- [ ] memory optimization for stateless dependencies
- [ ] memory optimization for dependencies (only store cached result and any stateful transitive dependencies

# Presentations
- [LEVEL UP 2021 Rome Developer Conference](https://github.com/erikvalkering/pigro_presentation_levelup2021/releases/download/v1/presentation.pdf) ([profile page](https://levelup.aiv01.it/EN/2021/84/Erik_Valkerin/888))
