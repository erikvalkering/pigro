#pragma once

namespace pigro {

// Credits go to Louis Dionne's Dyno library:
// https://github.com/ldionne/dyno/blob/03eaeded898225660787f03655edb89642a72e7c/include/dyno/detail/empty_object.hpp

template<typename T>
struct empty_object {
    struct T1 {
        char c;
    };
    struct T2 : T {
        char c;
    };

    union Storage {
        constexpr Storage() : t1{} {}
        T1 t1;
        T2 t2;
    };

    static T get() {
        Storage storage{};

        const auto *c = &storage.t2.c;

        auto t2 = reinterpret_cast<const T2 *>(c);
        auto t = static_cast<const T *>(t2);

        return *t;
    }
};

} // namespace pigro
