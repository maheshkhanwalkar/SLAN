PROJECT(slan)
CMAKE_MINIMUM_REQUIRED(VERSION 3.10)

SET(CMAKE_C_FLAGS "-Wall -Wextra -std=c99 -pedantic")

SET(SRC src/eth.c test/frame.c include/eth.h include/defs.h)
INCLUDE_DIRECTORIES(include)

ADD_EXECUTABLE(slan ${SRC})
FIND_PACKAGE(ZLIB REQUIRED)

if(ZLIB_FOUND)
    TARGET_LINK_LIBRARIES(slan ${ZLIB_LIBRARIES})
endif(ZLIB_FOUND)