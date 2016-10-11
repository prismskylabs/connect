set(JSON_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/json/src PARENT_SCOPE)

set(BUILD_CPR_TESTS OFF CACHE BOOL "Don't build cpr tests")
set(USE_SYSTEM_CURL OFF CACHE BOOL "Don't use system curl")

add_subdirectory(cpr)
