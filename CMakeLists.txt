cmake_minimum_required(VERSION 2.8.9)
project(skywATCh)
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_MACOSX_RPATH ON)

add_definitions(-DAPL=1)
add_definitions(-DXPLM200)
add_definitions(-DXPLM210)
add_definitions(-DXPLM300)
add_definitions(-DXPLM301)

find_library(COREFOUNDATION CoreFoundation)
find_library(OPENGL OpenGL)
find_library(XPLM_
 						 NAMES XPLM
						 HINTS "include/SDK/Libraries/Mac/")
find_library(XPWIDGETS_
						 NAMES XPWidgets
						 HINTS "include/SDK/Libraries/Mac/")

add_library(skywATCh SHARED src/skywATCh.cpp src/texture.cpp)
target_include_directories(skywATCh PRIVATE include include/SDK/CHeaders/XPLM include/SDK/CHeaders/Widgets)
target_link_libraries(skywATCh
    ${COREFOUNDATION}
		${OPENGL}
		${XPLM_}
		${XPWIDGETS_}
    )
set_target_properties(skywATCh PROPERTIES SUFFIX ".xpl")
SET_TARGET_PROPERTIES(skywATCh PROPERTIES PREFIX "")
install(TARGETS skywATCh DESTINATION /Users/kameroneves/Library/Application\ Support/Steam/steamapps/common/X-Plane\ 11/Resources/plugins/skywATCh/mac_x64)
