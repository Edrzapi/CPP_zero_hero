# tasks: the Library Manager, 12 tasks and a bonus, the same in
# every Zero to Hero repository (see tasks/README.md).
#
# Every task folder builds two programs:
#   taskNN_starter    builds and runs as it is; unfinished parts
#                     print "not implemented"
#   taskNN_solution   Core + Stretch
#
# The sources are picked up with a glob, so a .cpp file you add
# to a starter folder (task 03 Stretch 1, for example) is built
# on the next cmake --build without editing this file.
#
# ctest registers:
#   taskNN_starter    runs the starter, passes if it prints
#                     "not implemented"
#   check_taskNN      runs tasks/check/check.py on the solution
#                     (only when Python 3.10 or later is found)

find_package(Python3 3.10 COMPONENTS Interpreter)
if(NOT Python3_FOUND)
    message(STATUS "tasks: Python 3.10+ not found, the check_task tests are skipped")
endif()

set(ZH_TASKS_DATA  ${CMAKE_CURRENT_SOURCE_DIR}/data)
set(ZH_TASKS_CHECK ${CMAKE_CURRENT_SOURCE_DIR}/check/check.py)

# ------------------------------------------------------------
# zh_add_task(<folder> <number> [starter test args...])
# ------------------------------------------------------------
# Builds task<number>_starter and task<number>_solution from the
# .cpp files in <folder>/starter and <folder>/solution. Any extra
# arguments are passed to the starter when ctest runs it.

function(zh_add_task folder number)
    foreach(kind starter solution)
        file(GLOB sources CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${folder}/${kind}/*.cpp)
        set(target task${number}_${kind})
        add_executable(${target} ${sources})
        target_include_directories(${target} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/${folder}/${kind})
        zh_apply_settings(${target})
    endforeach()
    add_test(NAME task${number}_starter COMMAND task${number}_starter ${ARGN}
             WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
    set_tests_properties(task${number}_starter PROPERTIES PASS_REGULAR_EXPRESSION "not implemented" TIMEOUT 60)
endfunction()

# ------------------------------------------------------------
# zh_add_task_check(<check id> <target> [args...])
# ------------------------------------------------------------
# Runs the checker on a program. The working directory is the
# build folder, so tasks 08 and 10 write their out/ folder there.

function(zh_add_task_check id target)
    if(Python3_FOUND)
        add_test(NAME check_task${id}
                 COMMAND ${Python3_EXECUTABLE} ${ZH_TASKS_CHECK} ${id} -- $<TARGET_FILE:${target}> ${ARGN}
                 WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
    endif()
endfunction()

# ------------------------------------------------------------
# The Tasks
# ------------------------------------------------------------

zh_add_task(01_bookshelf 01)
zh_add_task_check(01 task01_solution)

zh_add_task(02_search_and_report 02)
zh_add_task_check(02 task02_solution)

zh_add_task(03_functions 03)
zh_add_task_check(03 task03_solution)

zh_add_task(04_dynamic_catalogue 04)
zh_add_task_check(04 task04_solution)

zh_add_task(05_classes 05)
zh_add_task_check(05 task05_solution)

zh_add_task(06_book_types 06)
zh_add_task_check(06 task06_solution)

zh_add_task(07_errors 07)
zh_add_task_check(07 task07_solution)

zh_add_task(08_persistence 08 ${ZH_TASKS_DATA})
zh_add_task_check(08 task08_solution ${ZH_TASKS_DATA})

zh_add_task(09_due_dates 09 ${ZH_TASKS_DATA})
zh_add_task_check(09 task09_solution ${ZH_TASKS_DATA})

# Task 10 uses the operating system's SQLite, never a download:
# winsqlite3 (part of Windows 10 and 11, header and .lib in the
# Windows SDK) with MSVC, the system libsqlite3 elsewhere
# (macOS has it; on Linux: sudo apt install libsqlite3-dev).
if(MSVC)
    set(ZH_SQLITE_LIB winsqlite3)
else()
    find_package(SQLite3)
    if(SQLite3_FOUND)
        set(ZH_SQLITE_LIB SQLite::SQLite3)
    endif()
endif()

if(ZH_SQLITE_LIB)
    zh_add_task(10_sqlite 10 ${ZH_TASKS_DATA})
    target_link_libraries(task10_starter PRIVATE ${ZH_SQLITE_LIB})
    target_link_libraries(task10_solution PRIVATE ${ZH_SQLITE_LIB})
    zh_add_task_check(10 task10_solution ${ZH_TASKS_DATA})
else()
    message(STATUS "tasks: SQLite not found, task 10 is skipped (install libsqlite3-dev)")
endif()

# Task 11 uses the repository's own test harness, testing/mini_test.h
zh_add_task(11_testing 11)
target_include_directories(task11_starter PRIVATE ${PROJECT_SOURCE_DIR}/testing)
target_include_directories(task11_solution PRIVATE ${PROJECT_SOURCE_DIR}/testing)
zh_add_task_check(11 task11_solution)

# Task 12: raw sockets. Windows needs the Winsock library.
zh_add_task(12_library_api 12 ${ZH_TASKS_DATA} 0)
if(WIN32)
    target_link_libraries(task12_starter PRIVATE ws2_32)
    target_link_libraries(task12_solution PRIVATE ws2_32)
endif()
zh_add_task_check(12 task12_solution ${ZH_TASKS_DATA} {port})
zh_add_task_check(12_stretch task12_solution --summarise ${ZH_TASKS_DATA}/openlibrary_dune.json)

# Task B: threads need the platform thread library on some systems
find_package(Threads REQUIRED)
zh_add_task(B_busy_library B)
target_link_libraries(taskB_starter PRIVATE Threads::Threads)
target_link_libraries(taskB_solution PRIVATE Threads::Threads)
zh_add_task_check(B taskB_solution)
