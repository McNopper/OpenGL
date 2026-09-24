# ============================================================
# warnings.cmake - compiler warning + runtime sanitizer configuration
#
# Single source of truth for the warning level and the optional sanitizer
# lane. Included from the top-level CMakeLists.txt and applied per target with
# project_enable_warnings(<target>...), or across targets created later in the
# current directory with project_enable_warnings_here().
#
# WHY THE DEFAULT IS "CHANGE NOTHING": this code is long established and
# widely built. With every option below at its default this file contributes
# exactly zero extra flags, so a clean checkout builds identically to before it
# existed and stays warning-free. The bug-elimination lane turns the options
# on in a scratch build directory; it never changes what the default build
# produces.
#
# WHY PER-TARGET, NEVER A BLIND GLOBAL: these projects fetch dependencies at
# configure time and compile generated code. A global add_compile_options()
# placed too early pushes the warning level onto that code as well and buries
# our own diagnostics in third-party noise. Applying warnings to our own
# targets keeps everything else building exactly as before, so every warning
# that appears is ours to fix.
#
# Honors:
#   * ENABLE_STRICT_WARNINGS (default OFF)
#         /W4      (MSVC)  -Wall -Wextra -Wpedantic (GCC, Clang)
#   * ENABLE_WERROR         (default OFF)
#         /WX      (MSVC)  -Werror                 (GCC, Clang)
#   * ENABLE_SANITIZER      (default "" = off; GCC and Clang only)
#         address / undefined / address,undefined / thread
# ============================================================

# Idempotent: this file may be reached both directly and through a project
# that vendors this one.
if(NOT COMMAND project_enable_warnings)

option(ENABLE_STRICT_WARNINGS
    "Raise the warning level (/W4, -Wall -Wextra -Wpedantic)" OFF)
option(ENABLE_WERROR "Treat compiler warnings as errors (/WX, -Werror)" OFF)
set(ENABLE_SANITIZER "" CACHE STRING
    "Runtime sanitizer to build with (address / undefined / address,undefined / thread); empty = off")

# Append the configured warning flags to the named list variable.
macro(_project_warning_flags out_var)
    set(${out_var} "")
    if(ENABLE_STRICT_WARNINGS)
        if(MSVC)
            list(APPEND ${out_var} /W4)
        else()
            list(APPEND ${out_var} -Wall -Wextra -Wpedantic)
        endif()
    endif()
    if(ENABLE_WERROR)
        if(MSVC)
            list(APPEND ${out_var} /WX)
        else()
            list(APPEND ${out_var} -Werror)
        endif()
    endif()
endmacro()

macro(_project_sanitizer_flags out_var)
    set(${out_var} "")
    if(ENABLE_SANITIZER)
        if(MSVC)
            message(WARNING "ENABLE_SANITIZER is not supported by MSVC - ignored")
        else()
            list(APPEND ${out_var}
                -fsanitize=${ENABLE_SANITIZER} -fno-omit-frame-pointer)
        endif()
    endif()
endmacro()

function(project_enable_warnings)
    foreach(_target IN LISTS ARGN)
        if(NOT TARGET ${_target})
            message(FATAL_ERROR "project_enable_warnings: no such target '${_target}'")
        endif()

        _project_warning_flags(_warn)
        if(_warn)
            target_compile_options(${_target} PRIVATE ${_warn})
        endif()

        _project_sanitizer_flags(_san)
        if(_san)
            target_compile_options(${_target} PRIVATE ${_san})
            # Passed as a link item rather than via target_link_options so this
            # file keeps working on the oldest CMake in use here.
            target_link_libraries(${_target} PRIVATE ${_san})
        endif()
    endforeach()
endfunction()

function(project_enable_warnings_here)
    # Directory scope: affects targets created AFTER this call in the current
    # directory and its subdirectories. Call it only once the dependency setup
    # is done, so fetched and generated code is not affected. Prefer
    # project_enable_warnings(<target>) where target names are known - it
    # cannot leak into anything else.
    _project_warning_flags(_warn)
    if(_warn)
        add_compile_options(${_warn})
    endif()

    _project_sanitizer_flags(_san)
    if(_san)
        add_compile_options(${_san})
        if(COMMAND add_link_options)
            add_link_options(${_san})
        else()
            add_link_libraries(${_san})
        endif()
    endif()
endfunction()

endif()
