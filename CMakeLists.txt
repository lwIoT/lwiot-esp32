cmake_minimum_required(VERSION 3.0)
project(lwiot-esp32 C CXX ASM)

include(CMakeToolsHelpers OPTIONAL)
include(CheckIncludeFiles)
include(CheckTypeSize)
include(CheckCSourceCompiles)
include(TestBigEndian)

if (CMAKE_BUILD_TYPE MATCHES Debug)
    set(HAVE_DEBUG True)
endif ()

SET(CMAKE_CXX_FLAGS_RELEASE "-O3")
SET(CMAKE_C_FLAGS_RELEASE "-O3")
SET(CMAKE_CXX_FLAGS_DEBUG "-ggdb -g3")
SET(CMAKE_C_FLAGS_DEBUG "-ggdb -g3")
SET(CMAKE_CXX_FLAGS_MINSIZEREL "-Os")
SET(CMAKE_C_FLAGS_MINSIZEREL  "-Os")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CFLAGS_EXTRA} ${PORT_C_FLAGS} -Wall -Wextra -Wno-unused-parameter -Wno-sign-compare")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CFLAGS_EXTRA} ${PORT_CXX_FLAGS} -fstrict-enums -Wall -Wextra -Wno-unused-parameter -Wno-sign-compare")
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_C_STANDARD 11)

CHECK_INCLUDE_FILES (stdlib.h HAVE_STDLIB_H)
CHECK_INCLUDE_FILES (stdio.h  HAVE_STDIO_H)
CHECK_INCLUDE_FILES (stdint.h HAVE_STDINT_H)
CHECK_INCLUDE_FILES (stdarg.h HAVE_STDARG_H)
CHECK_INCLUDE_FILES (assert.h HAVE_ASSERT_H)
CHECK_INCLUDE_FILES (string.h HAVE_STRING_H)
CHECK_INCLUDE_FILES (time.h HAVE_TIME_H)
CHECK_INCLUDE_FILES (Winsock2.h HAVE_WINSOCK_H)
CHECK_INCLUDE_FILES (arpa/inet.h HAVE_INET_H)

CHECK_C_SOURCE_COMPILES (
        "
    #include <stdlib.h>
    #include <stdint.h>
    int main(int argc, char* argv[])
    {
        ssize_t size;
        (void)size;
        return 0;
    }
  " HAVE_SSIZE_T)

CHECK_C_SOURCE_COMPILES (
        "
    #include <stdlib.h>
    #include <stdint.h>
    int main(int argc, char* argv[])
    {
        size_t size;
        (void)size;
        return 0;
    }
  " HAVE_SIZE_T)

include(${PROJECT_SOURCE_DIR}/cmake/freertos-type-check.cmake)

add_subdirectory(lwiot-core)
add_subdirectory(esp32)