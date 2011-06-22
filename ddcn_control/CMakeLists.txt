
project(ddcn_control)
cmake_minimum_required(VERSION 2.4.0)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR}/../bin)

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
else(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wno-unused-parameter -Woverloaded-virtual")
endif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

find_package(Qt4 COMPONENTS QtCore QtDBus QtGui REQUIRED)

include(${QT_USE_FILE})

set(SRC
	MainWindow.cpp
	main.cpp
	SettingsDialog.cpp
	OnlinePeerModel.cpp
	OnlineGroupModel.cpp
)

set(MOC_H
	MainWindow.h
	SettingsDialog.h
)

QT4_WRAP_CPP(MOC_SRC ${MOC_H})

set(QRC
	icons.qrc
)

QT4_ADD_RESOURCES(QRC_SRC ${QRC})

set(UIS
	MainWindow.ui
	SettingsDialog.ui
)

QT4_WRAP_UI(UIS_H ${UIS})

include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_executable(ddcn_control ${SRC} ${UIS_H} ${MOC_SRC} ${QRC_SRC})
target_link_libraries(ddcn_control ${QT_LIBRARIES})
