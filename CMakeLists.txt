cmake_minimum_required(VERSION 2.8.9 FATAL_ERROR)

project(swaggerfs C CXX)
set(target swaggerfs)

set(CMAKE_CXX_STANDARD 11)

# 
# Add Boost libraries
# 
find_package(Boost REQUIRED 
             COMPONENTS program_options filesystem unit_test_framework
                        system)

# 
# Add Fuse libraries from pkg-config database
# 
find_package(PkgConfig)
pkg_check_modules(FUSE fuse)

# 
# Set global defines
# 
add_definitions(
    ${FUSE_DEFINITIONS}
    -DFUSE_USE_VERSION=26
    -D_FILE_OFFSET_BITS=64
)

include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${Boost_INCLUDE_DIRS}
    ${FUSE_INCLUDE_DIRS}
)

set(libs
    ${Boost_FILESYSTEM_LIBRARY}
    ${Boost_PROGRAM_OPTIONS_LIBRARY}
    ${Boost_SYSTEM_LIBRARY}
    ${FUSE_LIBRARIES}
)

set(SRC "${CMAKE_SOURCE_DIR}/src")
set(src
    ${SRC}/main.cc
    ${SRC}/options.cc
    ${SRC}/commands.cc
    ${SRC}/service_manager.cc
    ${SRC}/swagger.cc
    ${SRC}/fuse.cc
    ${SRC}/http_client.cc
)

add_executable(${target} ${src})

target_link_libraries(${target} ${libs})


## 
## Unit test targets
## 
enable_testing()

# 
# Swagger tests
# 
set(TEST_SRC "${CMAKE_SOURCE_DIR}/test")
add_executable(test_swagger 
                ${TEST_SRC}/test_swagger_model.cc
                ${SRC}/swagger.cc
                ${SRC}/http_client.cc
               )
target_link_libraries(test_swagger ${libs}  
                                   ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
add_test(test_swagger test_swagger)

# 
# HTTP Client tests
# 
add_executable(test_http_client 
                ${TEST_SRC}/test_http_client.cc
                ${SRC}/swagger.cc
                ${SRC}/http_client.cc
               )
target_link_libraries(test_http_client ${libs}  
                                       ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
add_test(test_http_client test_http_client)



