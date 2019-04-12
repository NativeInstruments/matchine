cmake_minimum_required( VERSION 3.2 )

# ------------------------------------------------------------------------------
# PROJECT SETUP
# ------------------------------------------------------------------------------

project( matchine )

if( CMAKE_PROJECT_NAME STREQUAL "matchine" )
    option( MATCHINE_TESTS "Build matchine tests" ON )
else()
    option( MATCHINE_TESTS "Build matchine tests" OFF )
endif()


# ------------------------------------------------------------------------------
# DEPENDENCIES
# ------------------------------------------------------------------------------

find_package( Boost )
find_package( Gtest QUIET )
find_package( Nonius QUIET )


# ------------------------------------------------------------------------------
# LIBRARY
# ------------------------------------------------------------------------------

add_library( matchine INTERFACE )
target_compile_features( matchine INTERFACE cxx_std_14 )
target_include_directories( matchine INTERFACE include )


# ------------------------------------------------------------------------------
# TESTS
# ------------------------------------------------------------------------------

if( MATCHINE_TESTS )

    set(files_test
        tests/main.cpp
        tests/match.test.cpp
        tests/match_any.test.cpp
        tests/meta.test.cpp
        tests/overload.test.cpp
        tests/signature.test.cpp
        tests/type_hierarchy.test.cpp
    )

    add_executable( matchine_tests ${files_test} )
    set_target_properties( matchine_tests PROPERTIES CXX_STANDARD 17 )
    target_link_libraries( matchine_tests PRIVATE matchine gtest )

endif()

# ------------------------------------------------------------------------------
# BENCHMARKS
# ------------------------------------------------------------------------------

if (Nonius)
    add_executable( benchmarks "benchmarks/dynamic_cast.cpp" )
    target_link_libraries( benchmarks LINK_PUBLIC matchine nonius )
else()
    message("<> Nonius not found, won't build benchmarks.")
endif()
