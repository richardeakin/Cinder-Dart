cmake_minimum_required( VERSION 3.1 )
project( cinder-dart )

# Cache options
set( CINDER_PATH "../cinder" CACHE STRING "Path to Cinder directory" )

set( CMAKE_VERBOSE_MAKEFILE ON )
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14" )

get_filename_component( CINDER_PATH_FULL "${CMAKE_CURRENT_SOURCE_DIR}/cinder" ABSOLUTE )

message( STATUS "cinder path: ${CINDER_PATH_FULL}" )

if( NOT CMAKE_BUILD_TYPE )
	set( CMAKE_BUILD_TYPE Debug CACHE STRING
		 "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel. "
		 FORCE
	)
else()
	message( STATUS "CMAKE_BUILD_TYPE set to ${CMAKE_BUILD_TYPE}" )
endif()

set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib/${CMAKE_BUILD_TYPE} )

set( DART_INCLUDE_USER
	${CINDER_PATH}/include
	${CMAKE_CURRENT_SOURCE_DIR}/src
)

set( DART_INCLUDE_SYSTEM
	${CINDER_PATH_FULL}/include
)

include_directories( ${DART_INCLUDE_USER} )
include_directories( ${DART_INCLUDE_SYSTEM} SYSTEM )

set( CIDART_SRC_PATH ${CMAKE_CURRENT_SOURCE_DIR}/src )

list( APPEND CIDART_SOURCES
	${CIDART_SRC_PATH}/cidart/Debug.h
	${CIDART_SRC_PATH}/cidart/Exception.cpp
	${CIDART_SRC_PATH}/cidart/Exception.h
	${CIDART_SRC_PATH}/cidart/Script.cpp
	${CIDART_SRC_PATH}/cidart/Script.h
	${CIDART_SRC_PATH}/cidart/Types.cpp
	${CIDART_SRC_PATH}/cidart/Types.h
	${CIDART_SRC_PATH}/cidart/VM.cpp
	${CIDART_SRC_PATH}/cidart/VM.h
	${CIDART_SRC_PATH}/cidart/dart.h
)

add_library( cinder-dart ${CIDART_SOURCES} )

