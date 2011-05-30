
project(ddcn_service)
cmake_minimum_required(VERSION 2.4.0)

set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wno-unused-parameter")

find_package(Qt4 COMPONENTS QtCore QtDBus REQUIRED)
include(${QT_USE_FILE})

set(SRC
	main.cpp
)

set(MOC_H
	CompilerService.h
	CompilerNetwork.h
	Job.h
	JobAdaptor.h
)

QT4_WRAP_CPP(MOC_SRC ${MOC_H})

#include_directories(${QT_INCLUDES})
add_executable(ddcn_service ${SRC} ${UIS_H})
target_link_libraries(ddcn_service ${QT_LIBRARIES} )

