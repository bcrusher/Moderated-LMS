cmake_minimum_required(VERSION 3.20)

project(ModeratedLMS VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(SQLite3 REQUIRED)
find_package(OpenSSL REQUIRED)

add_executable(moderated_lms
    src/main.cpp
)

target_include_directories(moderated_lms PRIVATE include)

target_link_libraries(moderated_lms
    PRIVATE
    SQLite::SQLite3
    OpenSSL::Crypto
)
