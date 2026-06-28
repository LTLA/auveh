include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/ff233bdd4cac0a0bf6e5cd45bda3406814cb2796.zip
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Avoid installing GoogleTest when installing this project.
option(INSTALL_GTEST "Enable installation of googletest." OFF)

FetchContent_MakeAvailable(googletest)

enable_testing()

include(GoogleTest)

set(CODE_COVERAGE OFF CACHE BOOL "Enable coverage testing")
set(DO_CODE_COVERAGE OFF)
if(CODE_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(DO_CODE_COVERAGE ON)
endif()

macro(decorate_executable target)
    target_link_libraries(
        ${target}
        gtest_main
        auveh
    )

    target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_definitions(${target} PRIVATE SUBPAR_STRICT_SIGNATURES=1)

    if(DO_CODE_COVERAGE)
        target_compile_options(${target} PRIVATE -O0 -g --coverage)
        target_link_options(${target} PRIVATE --coverage)
    endif()

    gtest_discover_tests(${target})
endmacro()

add_executable(
    libtest
    src/ivdep.cpp
    src/width.cpp
)
decorate_executable(libtest)

add_executable(
    stdtest
    src/width.cpp
)
decorate_executable(stdtest)
set_property(TARGET stdtest PROPERTY CXX_STANDARD 26)
