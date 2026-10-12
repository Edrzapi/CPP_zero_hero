# io: iostreams, string streams, file streams, std::filesystem,
# std::format, std::print (C++23)

zh_add_example(io 01_iostream       01_iostream.cpp)
zh_add_example(io 02_string_streams 02_string_streams.cpp)
zh_add_example(io 03_fstream        03_fstream.cpp)
zh_add_example(io 04_filesystem     04_filesystem.cpp)
zh_add_example(io 05_format         05_format.cpp)

if(CPPZH_ENABLE_CXX23)
    zh_add_cxx23_example(io 06_print_cxx23 06_print_cxx23.cpp)
endif()

zh_add_topic_runner(io)
