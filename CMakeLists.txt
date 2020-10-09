cmake_minimum_required(VERSION 3.17)
project(homework C)

set(CMAKE_CXX_STANDARD 14)            # Enable c++14 standard

#add_executable(homework main.c cpu-limited-direct-execution/hw.c)

# Add executable target with source files listed in SOURCE_FILES variable
#add_executable(simple_example ${SOURCE_FILES})

# Add main.c file of project root directory as source file
set(SOURCE_FILES main.c cpu-limited-direct-execution/homework.c threads-cv/main-two-cvs-if.c threads-cv/homework.c)

# Add executable target with source files listed in SOURCE_FILES variable
add_executable(simple_example ${SOURCE_FILES})
