# Pigro
_Lazy evaluation on steroids_

Pigro is a C++20 library that allows you to define functions in a declarative and reactive way, resulting in code that is easier to reason about, easier to maintain, and less prone to errors.

# Lazy functions
Let's start out with the simplest use case: we want to make sure that some function is called at most once. Imagine we have several functions that use some global resources (e.g. an OpenGL rendering system) and a function that initializes those resources (e.g. `initialize_opengl()`). Before calling those functions that use the global resources, we must make sure that the `initialize_opengl()` function has been called. Furthermore, this should be done at most once, because otherwise it would reset the entire rendering system, undoing all of the rendering work that has been performed so far. Now also assume that for some reason we want to postpone this initialization as long as possible, because it might for example be a very heavy operation, and our application may not always need to use the rendering system. Therefore, simply calling `initialize_opengl()` on startup wouldn't be sufficient.

A typical solution would be to keep track of whether the `initialize_opengl()` function was called already, using a boolean flag, and only call it when this flag is set to `false`:

```c++
auto initialize_opengl() -> void;
auto is_initialized = false;

// ...at some point
if (should_draw_something) {
    if (!is_initialized) {
        initialize_opengl();
        is_initialized = true;
    }

    draw_something();
}

// ...elsewhere
if (should_draw_something_else) {
    if (!is_initialized) {
        initialize_opengl();
        is_initialized = true;
    }

    draw_something_else();
}

// etc.
```

Apart from the boilerplate, this ties an **implicit** relationship between the `initialize_opengl()` function and the `is_initialized` flag, which is a [code smell](https://en.wikipedia.org/wiki/Code_smell) because this knowledge lies only with the developer(s) and cannot be compiler-enforced. This would become even more of a problem if you would have several functions that need to be called at most once. In such cases, you'd need to keep track of a separate flag for each function. For example, consider what it would look like, if in addition to the rendering system, we now also have a compute system that needs to be initialized similarly (e.g. by a call to `initialize_opencl()`):

```c++
auto initialize_opengl() -> void;
auto initialize_opencl() -> void;
auto is_initialized_opengl = false;
auto is_initialized_opencl = false;

// ...at some point
if (should_draw_something) {
    if (!is_initialized_opengl) {
        initialize_opengl();
        is_initialized_opengl = true;
    }

    if (!is_initialized_opencl) {
        initialize_opencl();
        is_initialized_opencl = true;
    }

    const auto something = compute_something();
    draw(something);
}

// ...elsewhere
if (should_draw_something_else) {
    if (!is_initialized_opengl) {
        initialize_opengl();
        is_initialized_opengl = true;
    }

    if (!is_initialized_opencl) {
        initialize_opencl();
        is_initialized_opengl = true;
    }

    const auto something_else = compute_something_else();
    draw(something_else);
}

// etc.
```

As can be seen, code written in this way has a negative impact on the maintainability of it (in fact, there is a bug in the code).

Instead, it would be better to combine each initialization function and its corresponding flag into a single entity. This is where the `pigro::lazy()` function comes into play. Using it will wrap an existing function, and make sure that it will be called at most once:
```c++
auto ensure_initialized_opengl = pigro::lazy(initialize_opengl);
auto ensure_initialized_opencl = pigro::lazy(initialize_opencl);

// ...at some point
if (should_draw_something) {
    ensure_initialized_opengl();
    ensure_initialized_opencl();
    
    const auto something = compute_something();
    draw(something);
}

// ...elsewhere
if (should_draw_something_else) {
    ensure_initialized_opengl();
    ensure_initialized_opencl();
    
    const auto something_else = compute_something_else();
    draw(something_else);
}

// etc.
```

The resulting code is much cleaner: there is less boilerplate, but more importantly, the `is_initialized` flag is now maintained inside of the `ensure_initialized()` function, resulting in a better maintainable code. Additionally, the bug in the previously hand-written code has disappeared.

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

As can be seen, `lazy()` simply returns a new function that wraps `f()` and keeps track of the flag `is_called` and only calls `f()` when this flag is `false` and subsequently sets the flag to `true` such that the next time the lazy function is called, it will no longer call `f()`.

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
        if (!cache)
            cache = f();

        return *cache;
    };
}
```

Basically, the only change we made in order to support caching, was to change the `bool` flag to an `std::optional`, which holds both the cached value and the flag.

# Dependencies
In the previous two examples, the main use case for both was to ensure that the wrapped function is called at most once. However, in some cases an event might require that the wrapped function to be called again. Consider for example a graphical editor, with a rendering loop, in which we need to render a mouse cursor:

```c++
auto render_mouse_cursor(const point_2d pos, const image &icon) -> ui_object;
auto get_mouse_pos() -> point_2d;
auto load_image(const std::string_view filename) -> image;

// Rendering loop for a graphical editor
while (true) {
    render_mouse_cursor(get_mouse_pos(), load_image("arrow.png"));
}
```

As an optimization, we might want to skip the render call if the mouse was not moved. Unfortunately, simply wrapping the render call inside of `pigro::lazy()` won't help here, because that will render the mouse cursor **only** once, instead of only when the mouse is being moved. We'd somehow need to "invalidate" the cache or the `is_called` flag in case the mouse was moved.

Fortunately, the `pigro::lazy()` utility also has this use case covered, and this is actually where the Pigro library shines: Reactive Programming.

In addition to passing the render function, `pigro::lazy()` accepts additional _dependencies_ - _functions_ which are supposed to provide the inputs to the wrapped function:
```c++
auto render_mouse_cursor(const point_2d pos, const image &icon) -> ui_object;
auto get_mouse_pos() -> point_2d;
auto load_image(const std::string_view filename) -> image;

auto arrow = [] { return load_image("arrow.png"); };
auto mouse_cursor = pigro::lazy(render_mouse_cursor, get_mouse_pos, arrow);

// Rendering loop for a graphical editor
while (true) {
    mouse_cursor();
}
```

Now, when the lazy `mouse_cursor()` function is being called, it will first check whether any dependency has changed, and if so, only then it will call the actual wrapped function. As a result, the `render_mouse_cursor()` function will _only_ be called if the mouse was in fact moved (i.e. if `get_mouse_pos()` returned a different value). A look behind the scenes might give away the magic that is going on (again **heavily simplified**):

```c++
auto lazy(auto f, auto ...dependencies) {
    auto cache = std::optional<decltype(f())>{};
    auto dependencies_cache = std::optional<decltype(std::tuple{dependencies()})>{};

    return [=]() mutable {
        const auto args = std::tuple{dependencies()...};
        if (!cache || args != dependencies_cache) {
            cache = std::apply(f, args);
            dependencies_cache = args;
        }

        return *cache;
    };
}
```

This new version of the `pigro::lazy()` utility keeps track of two caches: one for the wrapped function, and another that bundles all of the return values from the dependencies. Now, in order to determine whether we should call our wrapped function, we simply check whether the function cache was previously filled, or whether any of the dependencies have changed, by comparing them against this second _dependencies_ cache.

Although this is slightly more complex than the previous version, it opens up nice new possibilities.

A keen eye may have noticed that there is some redundant work being done. Each time that the mouse is being moved, we render the mouse cursor. However, we are also loading the arrow image again and again, even though this image might not change.

Which the current functionality, this issue is easily fixed:
```c++
// ...
auto arrow = pigro::lazy([] { return load_image("arrow.png"); });
auto mouse_cursor = pigro::lazy(render_mouse_cursor, get_mouse_pos, arrow);
// ...
```

Now, the `arrow()` function can be used as dependency for the `render_mouse_cursor()` function, while at the same time being optimized to be called only once.

# Syntactic sugar
Because the previous pattern occurs quite often, i.e. having a constant-valued dependency that should be cached, there is a short-hand syntax available such that we can pass values directly as dependencies to the `pigro::lazy()` utility:
```c++
auto mouse_cursor = pigro::lazy(render_mouse_cursor, get_mouse_pos, load_image("arrow.png"));
```

This will wrap the image dependency with a lazy function, such that the value will be cached and used inside of the `render_mouse_cursor()` function, instead of loading the images every time.

In order to make this work, we actually don't need to modify the implementation of the existing `pigro::lazy()` utility, but merely constrain it a bit and add a new overload:
```c++
auto lazy(auto f, std::invocable auto ...dependencies) {
    // ...as before...
}

auto ensure_invocable(std::invocable auto dependency) {
     return dependency;
}

auto ensure_invocable(auto dependency) {
    return lazy([=] { return dependency; });
}

auto lazy(auto f, auto ...dependencies) {
    return lazy(f, ensure_invocable(dependencies...));
}
```

The first `lazy`-overload is the previous `lazy()` implementation, but now constrained using `std::invocable`, such that it will only be selected if all of the dependencies can be called. The second `lazy`-overload will be selected otherwise, and simply transforms each dependency into a form that can be invoked as a function, and then delegates to the first overload. The two `ensure_invocable()` overloads are helpers that perform this transformation. The first will be selected for dependencies that are already callable, and therefore returns them as-is. The second one creates a function that returns the dependency and subsequently wraps it with `lazy()` to make sure that it will be called only once (and therefore the cache will be used).

## Extra laziness
One drawback of passing the image as a value dependency, is that it will be evaluated, even in the case that it might never be called. For example, it the following case this would unnecessarily load resources from disk:
```c++
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
```c++
auto arrow = pigro::lazy(load_image, "arrow.png");
auto mouse_cursor = pigro::lazy(render_mouse_cursor, get_mouse_pos, arrow);
```

Now, the image won't even be loaded until when it is actually needed. Afterwards, the previously cached value will be used.

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

## Comparison with hand-coded solution
**Lazy & Reactive - using `pigro`**
```c++
auto mode = lazy(get_drawing_mode);
auto filename = lazy(get_mouse_icon_filename, mode);
auto icon = lazy(load_image, filename);
auto mouse_cursor = lazy(render_mouse_cursor, get_mouse_pos, icon);

// Rendering loop for a graphical editor
while (true) {
    mouse_cursor();
}
```

**Hand-coded**
```c++
auto should_render_mouse_cursor = false;
auto cache_pos = std::optional<point_2d>{};
auto cache_icon = std::optional<image>{};
auto cache_filename = std::optional<std::string>{};
auto cache_mode = std::optional<drawing_mode>{};

// Rendering loop for a graphical editor
while (true) {
    const auto pos = get_mouse_pos();
    if (cache_pos != pos) {
        cache_pos = pos;
        should_render_mouse_cursor = true;
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

                should_render_mouse_cursor = true;
            }
        }
    }

    if (should_render_mouse_cursor /* && cache_pos && cache_icon */) {
        render_mouse_cursor(*cache_pos, *cache_icon);
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
