add_executable(tests
	tests.cpp
	partition.test.cpp
	recursive_overload.test.cpp
	lazy.test.cpp
)
set_property(TARGET tests PROPERTY CXX_STANDARD 17)
