# ============================================================
# cppcheck.cmake - static analysis targets
#
# Self-contained: nothing here is required to build this project. If cppcheck
# is not installed the analysis targets are simply not registered and the
# build behaves exactly as before.
#
# Provides:
#   * cppcheck        - default profile, fails on findings (high signal/noise)
#   * cppcheck-strict - exhaustive profile (all + inconclusive), opt-in
#   * cppcheck-xml    - machine-readable XML report at
#                       ${CMAKE_BINARY_DIR}/reports/cppcheck/cppcheck.xml
#
# Honors:
#   * ENABLE_CPPCHECK            - master switch (default ON)
#   * ENABLE_CPPCHECK_IN_BUILD   - run alongside the compiler (default OFF)
# ============================================================

option(ENABLE_CPPCHECK "Register cppcheck static analysis targets" ON)
option(ENABLE_CPPCHECK_IN_BUILD "Run cppcheck alongside the compiler" OFF)

if(NOT ENABLE_CPPCHECK)
    return()
endif()

find_program(CPPCHECK_EXE NAMES cppcheck)

if(NOT CPPCHECK_EXE)
    message(STATUS "cppcheck not found - analysis targets not registered")
    return()
endif()

# The --project mode needs the compilation database in the binary dir.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "Export compile_commands.json" FORCE)

set(CPPCHECK_REPORT_DIR ${CMAKE_BINARY_DIR}/reports/cppcheck)
file(MAKE_DIRECTORY ${CPPCHECK_REPORT_DIR})

# Paths analysed by no profile: the build tree (which holds the fetched
# third-party sources), the checked-in assets and the rendered screenshots.
set(CPPCHECK_EXCLUDES
    -i${CMAKE_BINARY_DIR}
    -i${CMAKE_SOURCE_DIR}/build
    -i${CMAKE_SOURCE_DIR}/Binaries
    -i${CMAKE_SOURCE_DIR}/screenshots
)

# Default profile: high signal-to-noise. Fails the target on real findings.
# unusedFunction is deliberately NOT suppressed: every example is a
# self-contained application, so dead functions are exactly the defect class
# this gate exists to catch.
set(CPPCHECK_COMMON_ARGS
    --std=c11
    --enable=warning,performance,portability
    --inline-suppr
    --error-exitcode=1
    --quiet
    --template=gcc
    --suppressions-list=${CMAKE_SOURCE_DIR}/cppcheck.supp
    ${CPPCHECK_EXCLUDES}
)

# Opt-in exhaustive profile (style + inconclusive).
set(CPPCHECK_STRICT_ARGS
    --std=c11
    --enable=all
    --inconclusive
    --force
    --inline-suppr
    --error-exitcode=1
    --quiet
    --template=gcc
    --suppressions-list=${CMAKE_SOURCE_DIR}/cppcheck.supp
    ${CPPCHECK_EXCLUDES}
)

# Never collide with an identical target registered by a project that vendors
# this one (reached through add_subdirectory()/FetchContent): CMake fails hard
# on a duplicate add_custom_target name and would break that build outright.
if(TARGET cppcheck)
    message(STATUS "cppcheck targets already registered elsewhere - skipping")
    return()
endif()

add_custom_target(cppcheck
    COMMAND ${CPPCHECK_EXE}
        ${CPPCHECK_COMMON_ARGS}
        --project=${CMAKE_BINARY_DIR}/compile_commands.json
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Running cppcheck static analysis (default profile)"
    VERBATIM
)

add_custom_target(cppcheck-strict
    COMMAND ${CPPCHECK_EXE}
        ${CPPCHECK_STRICT_ARGS}
        --project=${CMAKE_BINARY_DIR}/compile_commands.json
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Running cppcheck static analysis (exhaustive: all + inconclusive)"
    VERBATIM
)

add_custom_target(cppcheck-xml
    COMMAND ${CPPCHECK_EXE}
        ${CPPCHECK_COMMON_ARGS}
        --xml
        --xml-version=2
        --project=${CMAKE_BINARY_DIR}/compile_commands.json
        2> ${CPPCHECK_REPORT_DIR}/cppcheck.xml
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Writing cppcheck XML report to ${CPPCHECK_REPORT_DIR}/cppcheck.xml"
    VERBATIM
)

if(ENABLE_CPPCHECK_IN_BUILD)
    set(CMAKE_C_CPPCHECK ${CPPCHECK_EXE} ${CPPCHECK_COMMON_ARGS}
        CACHE INTERNAL "cppcheck command used during compilation")
    message(STATUS "cppcheck enabled during build: ${CPPCHECK_EXE}")
else()
    message(STATUS "cppcheck targets enabled: ${CPPCHECK_EXE}")
endif()
