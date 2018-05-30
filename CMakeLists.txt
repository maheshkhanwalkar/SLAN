PROJECT(slan)
CMAKE_MINIMUM_REQUIRED(VERSION 3.10)

SET(CMAKE_C_FLAGS "-Wall -Wextra -std=gnu99 -pedantic")

SET(SRC src/eth.c test/valid_frame.c include/eth.h include/defs.h)
INCLUDE_DIRECTORIES(include)

ADD_EXECUTABLE(valid_frame ${SRC})
FIND_PACKAGE(ZLIB REQUIRED)

if(ZLIB_FOUND)
    TARGET_LINK_LIBRARIES(valid_frame ${ZLIB_LIBRARIES})
endif(ZLIB_FOUND)
