project(slan)
cmake_minimum_required(VERSION 3.10)

enable_testing()

set(CMAKE_C_COMPILER "clang")
set(CMAKE_C_FLAGS "-std=gnu99 -pedantic -Wall -Wextra -Wshadow")

set(CMAKE_C_FLAGS_DEBUG "-Og -glldb -Werror -fsanitize=address")
set(CMAKE_C_FLAGS_RELEASE "-O2 -march=native")

set(COMMON_SRC src/lib/eth.c include/eth.h include/defs.h
        src/lib/comm.c include/comm.h src/lib/msb.c include/msb.h)

include_directories(include)

add_test(
        NAME valid_frame
        COMMAND valid_frame
)

add_library(defs STATIC ${COMMON_SRC})

add_executable(valid_frame test/valid_frame.c)
add_executable(slan src/slan/slan.c)

find_package(ZLIB REQUIRED)

if(ZLIB_FOUND)
    target_link_libraries(valid_frame ${ZLIB_LIBRARIES} defs)
    target_link_libraries(slan ${ZLIB_LIBRARIES} defs)
endif(ZLIB_FOUND)
