
project(ddcn_crypto)
cmake_minimum_required(VERSION 2.4.0)

find_package(OpenSSL REQUIRED)

set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wno-unused-parameter")

find_package(Qt4 COMPONENTS QtCore REQUIRED)
include(${QT_USE_FILE})

set(SRC
	TLS.cpp
	PublicKey.cpp
	Certificate.cpp
	PublicKeyData.cpp
)

set(MOC_H
	TLS.h
)

QT4_WRAP_CPP(MOC_SRC ${MOC_H})

include_directories(${OPENSSL_INCLUDE_DIR})

add_library(ddcn_crypto SHARED ${SRC} ${MOC_SRC})
target_link_libraries(ddcn_crypto ${QT_LIBRARIES} ${OPENSSL_LIBRARIES})
