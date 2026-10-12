# under_the_hood: lesser-known and lower-level material.
# constexpr/consteval, structured bindings, variant/any,
# string_view lifetimes, attributes, user-defined literals,
# placement new and alignment, copy elision, small buffer
# optimisation, compiler flags and Compiler Explorer, and an
# opt-in undefined behaviour lab for the sanitizers.

zh_add_example(under_the_hood 01_constexpr_and_consteval   01_constexpr_and_consteval.cpp)
zh_add_example(under_the_hood 02_structured_bindings       02_structured_bindings.cpp)
zh_add_example(under_the_hood 03_variant_and_any           03_variant_and_any.cpp)
zh_add_example(under_the_hood 04_string_view_lifetimes     04_string_view_lifetimes.cpp)
zh_add_example(under_the_hood 05_attributes                05_attributes.cpp)
zh_add_example(under_the_hood 06_user_defined_literals     06_user_defined_literals.cpp)
zh_add_example(under_the_hood 07_placement_new_and_alignment 07_placement_new_and_alignment.cpp)
zh_add_example(under_the_hood 08_copy_elision              08_copy_elision.cpp)
zh_add_example(under_the_hood 09_small_buffer_optimisation 09_small_buffer_optimisation.cpp)
zh_add_example(under_the_hood 10_compiler_explorer         10_compiler_explorer.cpp)

if(CPPZH_BUILD_UB_DEMOS)
    zh_add_ub_demo(under_the_hood ub_examples ub_examples.cpp)
endif()

zh_add_topic_runner(under_the_hood)
