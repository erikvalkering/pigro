#pragma once

#include <concepts>
#include <optional>

namespace pigro {

struct regular_void {
    auto operator<=>(const regular_void &) const = default;
};

constexpr auto regularize_void(auto f) {
    return [=](auto... args) mutable {
        using result_t = decltype(f(args...));

        if constexpr (std::same_as<result_t, void>) {
            f(args...);
            return regular_void{};
        } else {
            return f(args...);
        }
    };
};

constexpr auto unregularize_void(auto value) {
    using value_t = std::remove_cvref_t<decltype(value)>;

    if constexpr (std::same_as<value_t, regular_void>) {
        return;
    } else {
        return value;
    }
};

} // namespace pigro

namespace std {

template<>
class optional<pigro::regular_void> {
    bool engaged = false;

public:
    optional() = default;
    constexpr optional(pigro::regular_void) : engaged{ true } {}

    constexpr auto &operator=(pigro::regular_void) {
        engaged = true;
        return *this;
    }

    constexpr explicit operator bool() const { return engaged; }
    constexpr auto operator*() const { return pigro::regular_void{}; }

    constexpr friend auto operator==(
      const optional<pigro::regular_void> lhs,
      const optional<pigro::regular_void> rhs) {
        return lhs.engaged == rhs.engaged;
    }

    constexpr friend auto operator!=(
      const optional<pigro::regular_void> lhs,
      const optional<pigro::regular_void> rhs) {
        return lhs.engaged != rhs.engaged;
    }
};

} // namespace std
