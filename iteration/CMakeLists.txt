# iteration: loops, range-for under the hood, generators (C++23)

zh_add_example(iteration 01_loops                     01_loops.cpp)
zh_add_example(iteration 02_range_for_under_the_hood  02_range_for_under_the_hood.cpp)

if(CPPZH_ENABLE_CXX23)
    zh_add_cxx23_example(iteration 03_generator_cxx23 03_generator_cxx23.cpp)
endif()

zh_add_topic_runner(iteration)
