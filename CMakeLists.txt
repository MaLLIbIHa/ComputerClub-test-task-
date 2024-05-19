cmake_minimum_required(VERSION 3.10)
project(ComputerClub)

set(SRC_LIST driver.cpp)

add_executable(${PROJECT_NAME} ${SRC_LIST})

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)