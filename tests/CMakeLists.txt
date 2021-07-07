add_executable(tests
	lazy.test.cpp
	overload.test.cpp
	partition.test.cpp
	recursive.test.cpp
	recursive_overload.test.cpp
	regular_void.test.cpp
	tests.cpp
	tuple_algorithms.test.cpp
)
set_property(TARGET tests PROPERTY CXX_STANDARD 20)
target_include_directories(tests PRIVATE ../extlib)
