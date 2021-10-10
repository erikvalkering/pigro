add_executable(tests
    tests.cpp

    apply.test.cpp
    bind_tuple.test.cpp
    compressed_tuple.test.cpp
    lazy.test.cpp
    overload.test.cpp
    pack_algorithms.test.cpp
    partition.test.cpp
    recursive.test.cpp
    regular_void.test.cpp
    to_tuple.test.cpp
    tuple_algorithms.test.cpp
    uncapture.test.cpp
    utils.test.cpp
)
set_property(TARGET tests PROPERTY CXX_STANDARD 20)
target_include_directories(tests PRIVATE ../extlib)
target_compile_options(tests PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -Werror>
)
