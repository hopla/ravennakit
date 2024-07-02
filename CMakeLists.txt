cmake_minimum_required(VERSION 3.22)

include(submodules/build_tools/cmake/git_version.cmake)

project(RavennaSDK VERSION ${GIT_VERSION_MAJOR}.${GIT_VERSION_MINOR}.${GIT_VERSION_PATCH} LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)

set(CMAKE_XCODE_ATTRIBUTE_DEBUG_INFORMATION_FORMAT "dwarf-with-dsym")
set(CMAKE_XCODE_ATTRIBUTE_DEPLOYMENT_POSTPROCESSING[variant=Release] "YES")
set(CMAKE_XCODE_ATTRIBUTE_DEPLOYMENT_POSTPROCESSING[variant=RelWithDebInfo] "YES")

option(WITH_ADDRESS_SANITIZER "Enable Address Sanitizer" OFF)
if (WITH_ADDRESS_SANITIZER)
    message("-- -DWITH_ADDRESS_SANITIZER=ON argument was specified: Enabling Clang's Address Sanitizer")
    add_compile_options(-fsanitize=address,undefined -g)
    link_libraries(-fsanitize=address,undefined)
endif ()

option(WITH_THREAD_SANITIZER "Enable Thread Sanitizer" OFF)
if (WITH_THREAD_SANITIZER)
    message("-- -DWITH_THREAD_SANITIZER=ON argument was specified: Enabling Clang's Thread Sanitizer")
    add_compile_options(-fsanitize=thread -g)
    link_libraries(-fsanitize=thread)
endif ()


#########
# VCPKG #
#########

find_package(spdlog CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(Catch2 CONFIG REQUIRED)

add_executable(ravenna_sdk_tests
        src/test/TestUnitTest.cpp
)

target_link_libraries(ravenna_sdk_tests
        PRIVATE
        Catch2::Catch2 Catch2::Catch2WithMain
)
