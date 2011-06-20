
project(ddcn_gcc)
cmake_minimum_required(VERSION 2.4.0)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR}/../bin)

set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wno-unused-parameter")

find_package(Qt4 COMPONENTS QtCore QtDBus REQUIRED)
include(${QT_USE_FILE})

set(SRC
	main.cpp
	Application.cpp
)

set(MOC_H
	Application.h
)

QT4_WRAP_CPP(MOC_SRC ${MOC_H})

add_executable(ddcn_gcc ${SRC} ${MOC_SRC})
target_link_libraries(ddcn_gcc ${QT_LIBRARIES})
