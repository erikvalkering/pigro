add_executable(tests
	bind_tuple.test.cpp
	compressed_tuple.test.cpp
	lazy.test.cpp
	overload.test.cpp
	pack_algorithms.test.cpp
	partition.test.cpp
	recursive.test.cpp
	regular_void.test.cpp
	tests.cpp
	tuple_algorithms.test.cpp
	uncapture.test.cpp
)
set_property(TARGET tests PROPERTY CXX_STANDARD 20)
target_include_directories(tests PRIVATE ../extlib)
