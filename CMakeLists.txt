cmake_minimum_required(VERSION 3.30)
project(analyzer VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(GTest REQUIRED)
find_package(range-v3 REQUIRED)
find_package(Boost REQUIRED COMPONENTS program_options)

include_directories(PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Сначала настраиваем библиотеки, которые будут подключаться к analyzer
add_subdirectory(src)

# Настраиваем сборку основного приложения analyzer
set(target analyzer)
add_executable(${target} main.cpp)

target_link_libraries(${target} 
    PRIVATE
        metric_accumulator
        metric
        cmd_options
        #range-v3::range-v3
)

# BUILD_INTERFACE автоматически подключает include-пути всех библиотек, указанных в target_link_libraries для analyzer
target_include_directories(${target}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

include(CTest)
enable_testing()
