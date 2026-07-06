cmake_minimum_required(VERSION 3.30)

project(Corral)

add_executable(Corral)

target_compile_features(Corral PRIVATE cxx_std_23)

file(GLOB_RECURSE MY_HEADERS "include/*.h")

target_sources(Corral
    PRIVATE
        main.cpp
    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS include
        FILES ${MY_HEADERS} 
)

target_compile_definitions(Corral PRIVATE
    $<$<CONFIG:Debug>:DEBUG _DEBUG>
    $<$<CONFIG:Release>:NDEBUG> 
)

target_compile_options(Corral PRIVATE
    $<$<CONFIG:Debug>:-g -O0 -fno-omit-frame-pointer -Wall -Werror>
    $<$<CONFIG:Release>:-O3 -Wall -Werror>
)

find_package(Boost REQUIRED)
target_include_directories(Corral PRIVATE ${Boost_INCLUDE_DIRS})