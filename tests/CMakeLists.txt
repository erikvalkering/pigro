add_executable(tests
	tests.cpp
	partition.test.cpp
	overload.test.cpp
	recursive.test.cpp
	recursive_overload.test.cpp
	lazy.test.cpp
)
set_property(TARGET tests PROPERTY CXX_STANDARD 20)
