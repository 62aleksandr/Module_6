# This file was automatically generated for projects
# without default 'CMakeLists.txt' file.

# FILE(GLOB_RECURSE app_sources ${CMAKE_SOURCE_DIR}/src/*.*)

# idf_component_register(SRCS ${app_sources})


file(GLOB_RECURSE app_sources "*.c")

idf_component_register(
    SRCS ${app_sources}
    INCLUDE_DIRS  "." "i2c" "tasks"        
    REQUIRES  onewire_bus ds18b20 ssd1306 i2cdev    
    )