cmake_minimum_required(VERSION 3.5)
project(ProjetLibGD LANGUAGES C)
find_package(gd)
add_executable(pie main.c)
target_link_libraries (pie m gd)
install(TARGETS pie
LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
