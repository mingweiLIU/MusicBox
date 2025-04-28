cmake_minimum_required(VERSION 3.16)

project(MusicBox VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenSceneGraph REQUIRED osg osgDB osgViewer osgGA osgUtil OpenThreads )
include_directories(${OPENSCENEGRAPH_INCLUDE_DIRS})

WTGetTypeFilesRelative(
    FileType "*.h"
    OutRelative HeaderFileNames
    Recurse OFF
)
WTGetTypeFilesRelative(
    FileType "*.cpp"
    OutRelative SourceFileNames
    Recurse OFF
)

WTGetTypeFilesRelative(
    FileType "*.png"
    OutRelative PNGFileNames
    Recurse ON
)

WTGetTypeFilesRelative(
    FileType "*.svg"
    OutRelative SVGFileNames
    Recurse ON
)

add_executable(${PROJECT_NAME} WIN32 ${SourceFileNames} ${HeaderFileNames}
    AppUI.h AppUI.cpp)
# set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_ENVIRONMENT "PATH=${OSG_DIR}/bin;$ENV{PATH}")
# get_target_property(DEBUG_ENV ${PROJECT_NAME} VS_DEBUGGER_ENVIRONMENT)
# message(STATUS "VS_DEBUGGER_ENVIRONMENT = ${DEBUG_ENV}")

set(includeDirs
    ${OPENSCENEGRAPH_INCLUDE_DIRS}
    ${BASS_DIR}
)

target_include_directories(${PROJECT_NAME} PUBLIC ${includeDirs})
target_link_libraries(${PROJECT_NAME}
    ${OPENSCENEGRAPH_LIBRARIES}
    ${BASS_DIR}/bass.lib    
    ${BASS_DIR}/basswasapi.lib
    dwmapi.lib
    dwrite.lib
    d2d1.lib
    winmm.lib
)

# 移除UNICODE定义（如果存在）
remove_definitions(-DUNICODE -D_UNICODE)

# 显式设置多字节字符集
add_definitions(-D_MBCS)

# 设置字符集相关编译选项
# target_compile_options(${PROJECT_NAME} PRIVATE
#     "/source-charset:gbk"          # 源文件使用GBK编码
#     "/execution-charset:gbk"       # 执行字符集使用GBK
# )

install(TARGETS ${PROJECT_NAME}
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

