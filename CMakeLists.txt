cmake_minimum_required(VERSION 3.10)
project(port_checker C)

# Find libuv package
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBUV REQUIRED libuv)

# Add executable
add_executable(server src/main.c)

# Link against libuv
target_link_libraries(server ${LIBUV_LIBRARIES})
target_include_directories(server PRIVATE ${LIBUV_INCLUDE_DIRS}) 