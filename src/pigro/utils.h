#pragma once

#define SFINAEABLE_RETURN(...) \
    ->decltype(__VA_ARGS__) {  \
        return __VA_ARGS__;    \
    }
