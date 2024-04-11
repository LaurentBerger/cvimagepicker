cmake_minimum_required(VERSION 3.2)


set(chapitre "cvimagepicker")

PROJECT (${chapitre})

find_package(OpenCV  REQUIRED)
cmake_policy(SET CMP0028 NEW)

file(GLOB SourceFile
    "*.h"
    "*.cpp")
ADD_EXECUTABLE (${chapitre} ${SourceFile})
target_compile_features(${chapitre} PRIVATE cxx_std_17)



if (OpenCV_FOUND)
	include_directories( ${OpenCV_INCLUDE_DIRS} )
	target_link_libraries( ${chapitre} ${OpenCV_LIBS} )
else (OpenCV_FOUND)
message("PB->OPENCV = ${OpenCV_INCLUDE_DIRS}")
endif (OpenCV_FOUND)
