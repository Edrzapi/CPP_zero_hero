# oop: classes, constructors, inheritance, virtual functions and
# the vtable, slicing, multiple/virtual inheritance, CRTP, PIMPL,
# operator overloading

zh_add_example(oop 01_classes               01_classes.cpp)
zh_add_example(oop 02_constructors          02_constructors.cpp)
zh_add_example(oop 03_inheritance           03_inheritance.cpp)
zh_add_example(oop 04_virtual_functions     04_virtual_functions.cpp)
zh_add_example(oop 05_object_slicing        05_object_slicing.cpp)
zh_add_example(oop 06_multiple_inheritance  06_multiple_inheritance.cpp)
zh_add_example(oop 07_crtp                  07_crtp.cpp)

# 08_pimpl: the Widget implementation is compiled into its own
# library, so main.cpp never sees the private details.
add_library(oop_pimpl_widget STATIC 08_pimpl/widget.cpp)
target_include_directories(oop_pimpl_widget PUBLIC 08_pimpl)
zh_apply_settings(oop_pimpl_widget)

zh_add_example(oop 08_pimpl 08_pimpl/main.cpp)
target_link_libraries(oop_08_pimpl PRIVATE oop_pimpl_widget)

zh_add_example(oop 09_operator_overloading  09_operator_overloading.cpp)

zh_add_topic_runner(oop)
