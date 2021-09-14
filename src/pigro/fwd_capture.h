#pragma once

namespace pigro::impl {

// Credits go to the article:
// "capturing perfectly-forwarded objects in lambdas", 11 december 2016
// https://vittorioromeo.info/index/blog/capturing_perfectly_forwarded_objects_in_lambdas.html

template<typename T>
class by_value {
private:
    T _x;

public:
    template<typename TFwd>
    by_value(TFwd &&x) : _x{ std::forward<TFwd>(x) } {
    }

    auto &get() & { return _x; }
    const auto &get() const & { return _x; }
    auto get() && { return std::move(_x); }
};

template<typename T>
class by_ref {
private:
    std::reference_wrapper<T> _x;

public:
    by_ref(T &x) : _x{ x } {
    }

    auto &get() & { return _x.get(); }
    const auto &get() const & { return _x.get(); }
    auto get() && { return std::move(_x.get()); }
};

template<typename T>
struct fwd_capture_wrapper : by_value<T> {
    using by_value<T>::by_value;
};

template<typename T>
struct fwd_capture_wrapper<T &> : by_ref<T> {
    using by_ref<T>::by_ref;
};

} // namespace pigro::impl

namespace pigro {

template<typename T>
auto fwd_capture(T &&x) {
    return impl::fwd_capture_wrapper<T>(std::forward<T>(x));
}

} // namespace pigro
