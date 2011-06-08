
project(ddcn_service)
cmake_minimum_required(VERSION 2.4.0)

set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wno-unused-parameter")

find_package(Qt4 COMPONENTS QtCore QtDBus REQUIRED)
include(${QT_USE_FILE})

set(SRC
	main.cpp
	CompilerNetwork.cpp
	CompilerNetworkAdaptor.cpp
	CompilerService.cpp
	CompilerServiceAdaptor.cpp
	Job.cpp
	JobAdaptor.cpp
	JobRequest.cpp
	OnlinePeer.cpp
	DBusStructs.cpp
)

set(MOC_H
	CompilerService.h
	CompilerServiceAdaptor.h
	CompilerNetwork.h
	CompilerNetworkAdaptor.h
	Job.h
	JobAdaptor.h
)

QT4_WRAP_CPP(MOC_SRC ${MOC_H})

add_executable(ddcn_service ${SRC} ${MOC_SRC})
target_link_libraries(ddcn_service ${QT_LIBRARIES} )
