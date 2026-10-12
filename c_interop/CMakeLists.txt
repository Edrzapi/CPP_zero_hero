# c_interop: C headers, extern "C", wrapping a C library in RAII,
# name mangling, struct layout and trivially copyable types

zh_add_example(c_interop 01_c_headers 01_c_headers.cpp)

# ------------------------------------------------------------
# 02_c_library: a C library compiled as C, used from C++
# ------------------------------------------------------------
# cstack.c is compiled by the C compiler (gcc / cl in C mode),
# because of its .c extension and the C in project(LANGUAGES C CXX).

add_library(c_interop_cstack STATIC 02_c_library/cstack.c)
target_include_directories(c_interop_cstack PUBLIC 02_c_library)
zh_apply_settings(c_interop_cstack)

zh_add_example(c_interop 02_c_library 02_c_library/main.cpp)
target_link_libraries(c_interop_02_c_library PRIVATE c_interop_cstack)

zh_add_example(c_interop 03_name_mangling      03_name_mangling.cpp)
zh_add_example(c_interop 04_structs_and_layout 04_structs_and_layout.cpp)

zh_add_topic_runner(c_interop)
