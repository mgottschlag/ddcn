
project(ddcn_service)
cmake_minimum_required(VERSION 2.4.0)

find_package(Boost REQUIRED COMPONENTS system)
SET(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
find_package(Ariba REQUIRED)
find_package(MCPO REQUIRED)
find_package(QCA2 REQUIRED)

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
	DBusStructs.cpp
	NetworkInterface.cpp
	McpoGroup.cpp
	BootstrapConfig.cpp
)

set(MOC_H
	CompilerService.h
	CompilerServiceAdaptor.h
	CompilerNetwork.h
	CompilerNetworkAdaptor.h
	JobRequest.h
	Job.h
	JobAdaptor.h
	NetworkInterface.h
	BootstrapConfig.h
)

QT4_WRAP_CPP(MOC_SRC ${MOC_H})

include_directories(${ARIBA_INCLUDE_DIR} ${MCPO_INCLUDE_DIR} ${Boost_INCLUDE_DIR} ${QCA2_INCLUDE_DIR})

add_executable(ddcn_service ${SRC} ${MOC_SRC})
target_link_libraries(ddcn_service ${QT_LIBRARIES} ${ARIBA_LIBRARY} ${PTHREAD_LIBRARY} ${MCPO_LIBRARY} ${Boost_LIBRARIES} ${QCA2_LIBRARY})
