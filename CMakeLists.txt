project(slan)
cmake_minimum_required(VERSION 3.10)

enable_testing()

set(CMAKE_C_COMPILER "clang")
set(CMAKE_C_FLAGS "-std=gnu99 -pedantic -Wall -Wextra -Wshadow")

set(CMAKE_C_FLAGS_DEBUG "-Og -glldb -Werror -fsanitize=address")
set(CMAKE_C_FLAGS_RELEASE "-O2 -march=native")

set(SRC src/eth.c test/valid_frame.c include/eth.h include/defs.h)
include_directories(include)

add_test(
        NAME valid_frame
        COMMAND valid_frame
)

add_executable(valid_frame ${SRC})
find_package(ZLIB REQUIRED)

if(ZLIB_FOUND)
    target_link_libraries(valid_frame ${ZLIB_LIBRARIES})
endif(ZLIB_FOUND)
