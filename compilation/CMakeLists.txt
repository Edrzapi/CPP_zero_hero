# compilation: the preprocessor, translation units, headers,
# declarations vs definitions, linkage and the ODR

zh_add_example(compilation 01_preprocessor 01_preprocessor.cpp)

# ------------------------------------------------------------
# 02_multi_file: a small static library plus a program using it
# ------------------------------------------------------------
# The library is compiled once into compilation_geometry.lib
# (Windows) or libcompilation_geometry.a (Linux/macOS), then
# linked into the executable. Only main.cpp is in the executable.

add_library(compilation_geometry STATIC
    02_multi_file/geometry.cpp
    02_multi_file/logger.cpp
    02_multi_file/inventory.cpp)
target_include_directories(compilation_geometry PUBLIC 02_multi_file)
zh_apply_settings(compilation_geometry)

zh_add_example(compilation 02_multi_file 02_multi_file/main.cpp)
target_link_libraries(compilation_02_multi_file PRIVATE compilation_geometry)

# ------------------------------------------------------------
# 03_odr_violation: opt-in, deliberately ill-formed
# ------------------------------------------------------------

if(CPPZH_BUILD_UB_DEMOS)
    zh_add_ub_demo(compilation 03_odr_violation
        03_odr_violation/main.cpp
        03_odr_violation/first.cpp
        03_odr_violation/second.cpp)
endif()

zh_add_topic_runner(compilation)
