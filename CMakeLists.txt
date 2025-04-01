cmake_minimum_required(VERSION 3.16)

project(MusicBox VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
set(CMAKE_CXX_STANDARD 17)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(atom)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/exe/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/exe/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/exe/lib)


set(ThirdPartyPath ${CMAKE_CURRENT_SOURCE_DIR}/3rd)
set(OSG_DIR ${ThirdPartyPath}/osg)
set(osgEarth_DIR ${ThirdPartyPath}/osg/cmake)
set(BASS_DIR ${ThirdPartyPath}/bass)

set(SubDirs
    App
)

#全局设置
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc  /Ob2")
link_directories(${CMAKE_ARCHIVE_OUTPUT_DIRECTORY})

foreach(oneDir ${SubDirs} )
    add_subdirectory(${oneDir})
endforeach( )