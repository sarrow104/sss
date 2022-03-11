cmake_minimum_required(VERSION 3.20)

#set(CMAKE_TOOLCHAIN_FILE $ENV{CMAKE_TOOLCHAIN_FILE})
project(unit-test)

include(GoogleTest) # for gtest_discover_tests()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_definitions(-DV8_COMPRESS_POINTERS)
add_definitions(-W -fexceptions -Wunused-variable -Wfatal-errors -Werror=return-type)

enable_testing()

#https://github.com/google/googletest/issues/702
find_package(GTest CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)

#######
#add_executable(unit-test-v8env v8env_tests.cpp ../src/detail/v8env.cpp)
#target_link_libraries(unit-test-v8env PRIVATE GTest::gmock GTest::gtest GTest::gmock_main GTest::gtest_main fmt::fmt-header-only v8 v8_libplatform sss iconv)
#add_test(NAME varlisp-gtest-v8env COMMAND unit-test-v8env)

file(GLOB mains RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

foreach(mainfile IN LISTS mains)

    # https://stackoverflow.com/questions/41315739/vcpkg-does-not-work-for-google-test

    # Get file name without directory and extension
    get_filename_component(mainname ${mainfile} NAME_WE)

    add_executable(${mainname} ${mainfile})

    #target_link_libraries(${mainname} PRIVATE GTest::gmock GTest::gtest GTest::gmock_main GTest::gtest_main fmt::fmt-header-only sss iconv)
    target_link_libraries(${mainname} PRIVATE GTest::gmock GTest::gtest GTest::gmock_main GTest::gtest_main fmt::fmt sss iconv)

    add_test(
        NAME ${mainname}
        COMMAND ${mainname}
        # tests are run in the directory containing the sources and the data 
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
    # get_test_property()

    # NOTE possible link_libraries
    # ${ss1x} ${sss}
    # uchardet mstch
    # ${Boost_LIBRARIES} gq gumbo multipart iconv Threads::Threads ${OPENSSL_LIBRARIES} z
endforeach()
