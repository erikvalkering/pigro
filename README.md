# pigro
_Lazy evaluation on steroids_

The Pigro library allows you to define functions in a declarative and reactive way, resulting in code that is easier to reason about, easier to maintain, and less prone to errors.

Let's start out with a simple example. Imagine we have some code that may use some database, but connecting to it is a relatively expensive operation. Therefore, you'd want to postpone the connection to the database until it is absolutely necessary.

Using the `pigro::lazy` utility, we can very easily create a function that just connects to the database the first time it is called and will reuse the previously created connection any subsequent time it is being called:

```c++
auto database = pigro::lazy([] {
    auto connection_string = ...;
    return connect_database(connection_string);
});
```

Behind the scenes, it will cache the return value and will return that for any subsequent call.

Now we can use it as follows:

```c++
auto foobar() {
   // ...
   database().query(...); // slow startup time due to connection
   //... 
   database().query(...); // fast using cached connection
}
```

# Features
- [x] easy creation of cached functions 
- [x] reactivity by specifying dependencies
- [x] support for values, functions, and lazy functions as dependency
- [x] skipping re-evaluation if dependencies evaluate to same values as previous ones
- [x] skipping comparison of lazy function dependencies if they were not re-evaluated
- [ ] memory optimization for stateless dependencies
- [ ] memory optimization for dependencies (only store cached result and any stateful transitive dependencies

# Presentations
- https://github.com/erikvalkering/pigro_presentation_levelup2021
