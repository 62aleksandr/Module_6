# This file was automatically generated for projects
# without default 'CMakeLists.txt' file.

FILE(GLOB_RECURSE app_sources ${CMAKE_SOURCE_DIR}/src/*.*)

idf_component_register(SRCS ${app_sources})


# idf_component_register(SRCS "queue_no_isr_project.c"
#                       INCLUDE_DIRS "."
#                       REQUIRES freertos
#                       PRIV_REQUIRES esp_common)