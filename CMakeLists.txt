if(WIN32)
cmake_minimum_required(VERSION 3.30)
else()
cmake_minimum_required(VERSION 3.22)
endif()

# Include directory to use with #include <...>
set(INCLUDE_DIRS "include")

# Sources and headers masks
set(SOURCES_INCLUDE_PATH *.cpp *.c *.cxx)
set(HEADERS_INCLUDE_PATH *.hpp *.h)

# Exclude list of files (regxp)
set(EXCLUDE_PATH "/res/|/opt/|/out/")

#-------------------------------------------------------

# Converting current folder name to PROJECT_NAME
string(REGEX REPLACE "/$" "" CURRENT_FOLDER_ABSOLUTE ${CMAKE_CURRENT_SOURCE_DIR})
string(REGEX REPLACE ".*/(.*)" "\\1" CURRENT_FOLDER ${CURRENT_FOLDER_ABSOLUTE})
set (PROJECT_NAME ${CURRENT_FOLDER})
# Set project name variable
project(${PROJECT_NAME})

#-------------------------------------------------------

# Compiler-specific flags

# Set C++ Flags
if(MSVC)
    # MSVC-specific flags
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /WX")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /permissive-")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /sdl")
else()
    set(CMAKE_CXX_STANDARD 20)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wfatal-errors")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic-errors")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-variable")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-parameter")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-but-set-variable")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-local-typedefs")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-function")
endif()

# Set C Flags
if(MSVC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /W4")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /WX")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /sdl")
else()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wextra")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wfatal-errors")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pedantic")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pedantic-errors")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused-variable")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused-parameter")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused-but-set-variable")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused-local-typedefs")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused-function")
endif()

#-------------------------------------------------------

#include search function .cmake file
include(cmake/search_sources.cmake)
# Search of all sources and headers files
file(GLOB_RECURSE SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/${SOURCES_INCLUDE_PATH})
file(GLOB_RECURSE HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/${HEADERS_INCLUDE_PATH})

# Filter sources and headers to build without ignore list
FilterRegex(EXCLUDE ${EXCLUDE_PATH} SOURCES ${SOURCES})
FilterRegex(EXCLUDE ${EXCLUDE_PATH} HEADERS ${HEADERS})
# Print the list of files collected
string(REPLACE ";"  "\n" HEADERS_STR "${HEADERS}")
string(REPLACE ";"  "\n" SOURCES_STR "${SOURCES}")
message(STATUS  "Found headers:")
list(APPEND CMAKE_MESSAGE_INDENT "\t\t")
message(STATUS  "${HEADERS_STR}")
list(POP_BACK CMAKE_MESSAGE_INDENT)
message(STATUS  "Found sources:")
list(APPEND CMAKE_MESSAGE_INDENT "\t\t")
message(STATUS  "${SOURCES_STR}")


#include boost lib
#include(cmake/boost.cmake)

#include boost lib
#include(cmake/openssl.cmake)

#include dirs for #include <...>
INCLUDE_DIRECTORIES(${INCLUDE_DIRS})

# Build executable with standart libs
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# threads package
find_package(Threads)
target_link_libraries(${PROJECT_NAME} PUBLIC ${CMAKE_THREAD_LIBS_INIT})

# Link winsock
if(WIN32)
TARGET_LINK_LIBRARIES(${PROJECT_NAME} LINK_PUBLIC ws2_32)
endif()

# Link boost
TARGET_LINK_LIBRARIES(${PROJECT_NAME} LINK_PUBLIC ${Boost_LIBRARIES})

# Link openssl
TARGET_LINK_LIBRARIES(${PROJECT_NAME} LINK_PUBLIC ${OPENSSL_LIBRARIES})