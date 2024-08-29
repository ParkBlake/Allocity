cmake_minimum_required(VERSION 3.12)
project(Allocity VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(SOURCES
    src/main.cpp
    src/DefaultAllocator.cpp
    src/Allocator.cpp
    src/AllocityHashtable.cpp
    src/AllocityThread.cpp
    src/MemoryPool.cpp
)

set(HEADERS
    include/Allocity_impl.hpp
    include/Allocator.hpp
    include/DefaultAllocator.hpp
    include/AllocityHashtable.hpp
    include/AllocityThread.hpp
    include/MemoryPool.hpp
)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

target_include_directories(${PROJECT_NAME} PRIVATE include)

find_package(Threads REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads)

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

add_custom_target(run_tests
    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}
    DEPENDS ${PROJECT_NAME}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Running tests for ${PROJECT_NAME}"
)
