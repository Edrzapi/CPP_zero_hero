# ============================================================
# Shared Build Helpers
# ============================================================
# Every topic folder uses these functions so that warnings,
# sanitiser flags and the run_<topic> targets stay identical
# across the whole repository.
# ============================================================

# ------------------------------------------------------------
# zh_apply_settings(<target> [NO_WERROR])
# ------------------------------------------------------------
# Warnings: -Wall -Wextra -Wpedantic on GCC and Clang, /W4 on
# MSVC. /permissive- turns off old MSVC extensions so MSVC
# behaves like the other two. /Zc:__cplusplus makes MSVC report
# the real language version in __cplusplus (it reports 199711L
# otherwise). /utf-8 reads source files as UTF-8. The two C++
# switches are applied to C++ sources only, so the C library in
# c_interop/ compiles cleanly with the same function.
# NO_WERROR leaves warnings as warnings even when
# CPPZH_WARNINGS_AS_ERRORS is ON (used by the UB demos).

function(zh_apply_settings target)
    set(werror ${CPPZH_WARNINGS_AS_ERRORS})
    if(ARGC GREATER 1 AND "${ARGV1}" STREQUAL "NO_WERROR")
        set(werror OFF)
    endif()
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4 /utf-8
            $<$<COMPILE_LANGUAGE:CXX>:/permissive- /Zc:__cplusplus>)
        # The C runtime marks fopen, localtime, strcpy and friends as
        # "unsafe". The c_interop and datetime modules teach those
        # functions on purpose, so the deprecation warnings are off.
        target_compile_definitions(${target} PRIVATE _CRT_SECURE_NO_WARNINGS)
        if(werror)
            target_compile_options(${target} PRIVATE /WX)
        endif()
        if(CPPZH_SANITIZE)
            # MSVC supports AddressSanitizer only (no UBSan, no TSan).
            # ASan is incompatible with incremental linking and with the
            # /RTC run-time checks (removed in the top-level CMakeLists).
            target_compile_options(${target} PRIVATE /fsanitize=address /Zi)
            target_link_options(${target} PRIVATE /INCREMENTAL:NO /DEBUG)
        endif()
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
        if(werror)
            target_compile_options(${target} PRIVATE -Werror)
        endif()
        if(CPPZH_SANITIZE)
            target_compile_options(${target} PRIVATE
                -fsanitize=address,undefined -fno-omit-frame-pointer -g)
            target_link_options(${target} PRIVATE -fsanitize=address,undefined)
        endif()
    endif()
endfunction()

# ------------------------------------------------------------
# zh_add_example(<topic> <name> <sources...>)
# ------------------------------------------------------------
# Creates executable <topic>_<name>, applies the settings,
# registers it with ctest and adds it to run_<topic>.

function(zh_add_example topic name)
    set(target ${topic}_${name})
    add_executable(${target} ${ARGN})
    zh_apply_settings(${target})
    add_test(NAME ${target} COMMAND ${target})
    set_property(GLOBAL APPEND PROPERTY ZH_TOPIC_${topic} ${target})
endfunction()

# ------------------------------------------------------------
# zh_add_cxx23_example(<topic> <name> <sources...>)
# ------------------------------------------------------------
# Same as zh_add_example but compiled as C++23. Only called when
# CPPZH_ENABLE_CXX23 is ON, because library support still varies
# (std::expected, std::print, std::generator, ranges::to).

function(zh_add_cxx23_example topic name)
    zh_add_example(${topic} ${name} ${ARGN})
    set_target_properties(${topic}_${name} PROPERTIES CXX_STANDARD 23)
endfunction()

# ------------------------------------------------------------
# zh_add_ub_demo(<topic> <name> <sources...>)
# ------------------------------------------------------------
# Deliberate undefined behaviour. Never built by default, never
# registered with ctest, never part of run_<topic>. Build with
# -DCPPZH_BUILD_UB_DEMOS=ON (and ideally -DCPPZH_SANITIZE=ON).

function(zh_add_ub_demo topic name)
    set(target ${topic}_${name})
    add_executable(${target} ${ARGN})
    # Compilers often warn about the bugs in these files, which is
    # part of the lesson. Keep them as warnings even under -Werror.
    zh_apply_settings(${target} NO_WERROR)
endfunction()

# ------------------------------------------------------------
# zh_add_topic_runner(<topic>)
# ------------------------------------------------------------
# Adds a run_<topic> target that builds then runs every example
# in the folder in file order:
#
#   cmake --build build --target run_basics

function(zh_add_topic_runner topic)
    get_property(targets GLOBAL PROPERTY ZH_TOPIC_${topic})
    set(commands "")
    foreach(t IN LISTS targets)
        list(APPEND commands
            COMMAND ${CMAKE_COMMAND} -E echo ""
            COMMAND ${CMAKE_COMMAND} -E echo "########## ${t} ##########"
            COMMAND $<TARGET_FILE:${t}>)
    endforeach()
    add_custom_target(run_${topic} ${commands} USES_TERMINAL VERBATIM)
    if(targets)
        add_dependencies(run_${topic} ${targets})
    endif()
endfunction()
