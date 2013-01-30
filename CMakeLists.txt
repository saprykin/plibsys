cmake_minimum_required(VERSION 2.8)
project(plib C)
set(VERSION "0.0.1")

if(!WIN32)
	message(FATAL_ERROR "CMake build system is only available for Windows targets")
endif()

subdirs(src)