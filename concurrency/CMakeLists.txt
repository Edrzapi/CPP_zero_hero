# concurrency: threads and jthread, mutexes and locks,
# condition variables, atomics, async and futures

# Threads need the platform thread library on some systems
# (pthread on older glibc). Threads::Threads handles that.
find_package(Threads REQUIRED)

foreach(example
        01_threads
        02_mutexes
        03_condition_variable
        04_atomics
        05_async_and_future)
    zh_add_example(concurrency ${example} ${example}.cpp)
    target_link_libraries(concurrency_${example} PRIVATE Threads::Threads)
endforeach()

# The Java course's RaceConditionRunner, done honestly: a data race
# on a plain int is undefined behaviour in C++, so it is opt-in.
if(CPPZH_BUILD_UB_DEMOS)
    zh_add_ub_demo(concurrency ub_data_race ub_data_race.cpp)
    target_link_libraries(concurrency_ub_data_race PRIVATE Threads::Threads)
endif()

zh_add_topic_runner(concurrency)
