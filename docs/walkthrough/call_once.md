# Call once
Let's start out with the simplest use case: we want to make sure that some function is called at most once. Imagine we have several functions that use some global resources (e.g. an OpenGL rendering system) and a function that initializes those resources (e.g. `initialize_opengl()`). Before calling those functions that use the global resources, we must make sure that the `initialize_opengl()` function has been called. Furthermore, this should be done at most once, because otherwise it would reset the entire rendering system, undoing all of the rendering work that has been performed so far. Now also assume that for some reason we want to postpone this initialization as long as possible, because it might for example be a very heavy operation, and our application may not always need to use the rendering system. Therefore, simply calling `initialize_opengl()` on startup wouldn't be sufficient.

A typical solution would be to keep track of whether the `initialize_opengl()` function was called already, using a boolean flag, and only call it when this flag is set to `false`:

```cpp
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

Apart from the boilerplate, this ties an **implicit** relationship between the `initialize_opengl()` function and the `is_initialized` flag, which is a [code smell](https://en.wikipedia.org/wiki/Code_smell) because this knowledge lies only with the developer(s) and therefore keeping them in-sync isn't compiler-enforced. This would become even more of a problem if you would have several functions that need to be called at most once. In such cases, you'd need to keep track of a separate flag for each function. For example, consider what it would look like, if in addition to the rendering system, we now also have a compute system that needs to be initialized similarly (e.g. by a call to `initialize_opencl()`):

```cpp
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
```cpp
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

The resulting code is much cleaner: there is less boilerplate, but more importantly, the `is_initialized` flag is now maintained inside of the `ensure_initialized()` function, resulting in a better maintainable code. Additionally, the bug in the previously handwritten code has disappeared (before, the `is_initialized_opengl` flag would be set after calling `initialize_opencl()`, instead of setting the `is_initialized_opencl` flag).

In order to better understand what is going on behind the scenes, the following is a **heavily simplified** version of the `pigro::lazy()` function:
```cpp
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
