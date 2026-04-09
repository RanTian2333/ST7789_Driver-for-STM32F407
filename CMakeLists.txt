#cmake_minimum_required(VERSION 3.22)
#
##
## This file is generated only once,
## and is not re-generated if converter is called multiple times.
##
## User is free to modify the file as much as necessary
##
#
## Setup compiler settings
#set(CMAKE_C_STANDARD 11)
#set(CMAKE_C_STANDARD_REQUIRED ON)
#set(CMAKE_C_EXTENSIONS ON)
#
#
## Define the build type
#if(NOT CMAKE_BUILD_TYPE)
#    set(CMAKE_BUILD_TYPE "Debug")
#endif()
#
## Set the project name
#set(CMAKE_PROJECT_NAME ST7789)
#
## Enable compile command to ease indexing with e.g. clangd
#set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)
#
## Core project settings
#project(${CMAKE_PROJECT_NAME})
#message("Build type: " ${CMAKE_BUILD_TYPE})
#
## Enable CMake support for ASM and C languages
#enable_language(C ASM)
#
## Create an executable object type
#add_executable(${CMAKE_PROJECT_NAME}
#        Core/Inc/lcd_st7789.h
#        Core/Src/lcd_st7789.c
#        Core/Src/gt30.c
#        Core/Inc/gt30.h)
#
## Add STM32CubeMX generated sources
#add_subdirectory(cmake/stm32cubemx)
#
## Link directories setup
#target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
#    # Add user defined library search paths
#)
#
## Add sources to executable
#target_sources(${CMAKE_PROJECT_NAME} PRIVATE
#    # Add user sources here
#)
#
## Add include paths
#target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
#    # Add user defined include paths
#)
#
## Add project symbols (macros)
#target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
#    # Add user defined symbols
#)
#
## Remove wrong libob.a library dependency when using cpp files
#list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)
#
## Add linked libraries
#target_link_libraries(${CMAKE_PROJECT_NAME}
#    stm32cubemx
#
#    # Add user defined libraries
#)


cmake_minimum_required(VERSION 3.22)

#
# This file is generated only once,
# and is not re-generated if converter is called multiple times.
#
# User is free to modify the file as much as necessary
#

# Set the project name
set(CMAKE_PROJECT_NAME ST7789)

# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Core project settings
project(${CMAKE_PROJECT_NAME} C ASM)

# Set C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

message("Build type: ${CMAKE_BUILD_TYPE}")
message("Compiler: ${CMAKE_C_COMPILER_ID}")

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME}
        Core/Inc/lcd_st7789.h
        Core/Src/lcd_st7789.c
        Core/Src/gt30.c
        Core/Inc/gt30.h
)

# 设置字符串字面量编码（仅 GCC/Clang）
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE -fexec-charset=GB2312)
endif()

# Add STM32CubeMX generated sources
add_subdirectory(cmake/stm32cubemx)

# Link directories setup (optional)
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
        # Add user defined library search paths here
)

# Add additional sources (optional)
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
        # Add user sources here
)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
        # Add user defined include paths here
)

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
        # Add user defined symbols here
)

# Remove wrong libob.a library dependency when using cpp files
list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
        stm32cubemx
        # Add user defined libraries here
)
