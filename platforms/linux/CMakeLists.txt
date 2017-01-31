include(../shared/shared.cmake)
findBoostCurl()
buildSdk()

add_subdirectory(../shared/tests)
add_subdirectory(../shared/examples)

