cmake_minimum_required(VERSION 3.10)

#set(CMAKE_CPP_COMPILER clang++)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(minirecoder)
set(target "${PROJECT_NAME}")
add_executable(${PROJECT_NAME} "")
target_sources(${target} PRIVATE "./main.c" "./serial.c") 
target_include_directories(${target} PRIVATE ".")

install(TARGETS ${target}
        RUNTIME DESTINATION "/usr/local/bin")
