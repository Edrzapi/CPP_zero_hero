# exceptions: try/catch, custom hierarchies, error codes vs
# exceptions, std::optional, noexcept and exception safety,
# std::expected (C++23)

zh_add_example(exceptions 01_exceptions                01_exceptions.cpp)
zh_add_example(exceptions 02_custom_exceptions         02_custom_exceptions.cpp)
zh_add_example(exceptions 03_error_codes_and_optional  03_error_codes_and_optional.cpp)
zh_add_example(exceptions 04_noexcept_and_safety       04_noexcept_and_safety.cpp)

if(CPPZH_ENABLE_CXX23)
    zh_add_cxx23_example(exceptions 05_expected_cxx23 05_expected_cxx23.cpp)
endif()

zh_add_topic_runner(exceptions)
