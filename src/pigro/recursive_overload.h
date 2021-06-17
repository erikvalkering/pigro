#pragma once

namespace pigro {

namespace detail {

template<typename... F>
struct recursive_overload : F... {
private:
    using F::operator()...;

protected:
    template<typename... Args>
    decltype(auto) call(Args &&...args) const {
        return (*this)(forward<Args>(args)...);
    }

    template<typename... Args>
    decltype(auto) call(Args &&...args) {
        return (*this)(forward<Args>(args)...);
    }
};

} // namespace detail

template<typename... F>
struct recursive_overload : detail::recursive_overload<F...> {
    template<typename... Args>
    decltype(auto) operator()(Args &&...args) const {
        return this->call(*this, forward<Args>(args)...);
    }

    template<typename... Args>
    decltype(auto) operator()(Args &&...args) {
        return this->call(*this, forward<Args>(args)...);
    }
};

template<typename... F>
recursive_overload(F...) -> recursive_overload<F...>;

} // namespace pigro
