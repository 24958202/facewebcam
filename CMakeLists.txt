# CMake configuration for FaceWebCam project  
cmake_minimum_required(VERSION 3.16)  

# Project name and version  
project(FaceWebCam VERSION 1.0 LANGUAGES CXX)  

# Set C++ standard  
set(CMAKE_CXX_STANDARD 20)  
set(CMAKE_CXX_STANDARD_REQUIRED ON)  

# Include directories for headers  
include_directories(include)  

# Add source files  
file(GLOB SOURCES ${CMAKE_SOURCE_DIR}/src/*.cpp)  

# Add the main program file  
set(MAIN_FILE ${CMAKE_SOURCE_DIR}/FaceWebCam.cpp)  

# Platform-specific settings  
if(WIN32)  
    # Windows-specific settings  
    set(INSTALL_BIN_DIR bin)  
    set(INSTALL_ICON_DIR icons)  
    set(INSTALL_DESKTOP_DIR .) # No desktop file on Windows  
elseif(APPLE)  
    # macOS-specific settings  
    set(INSTALL_BIN_DIR bin)  
    set(INSTALL_ICON_DIR share/icons)  
    set(MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/Info.plist)  
    set_source_files_properties(${MAIN_FILE} PROPERTIES MACOSX_PACKAGE_LOCATION "MacOS")  
    set(INSTALL_DESKTOP_DIR .) # No desktop file on macOS  
else()  
    # Linux-specific settings  
    set(INSTALL_BIN_DIR /usr/local/bin)  
    set(INSTALL_ICON_DIR /usr/local/share/icons)  
    set(INSTALL_DESKTOP_DIR /usr/local/share/applications)  
endif()  

# Add the executable  
add_executable(${PROJECT_NAME} ${MAIN_FILE} ${SOURCES} ${CMAKE_SOURCE_DIR}/facewebcam.xpm)  

# Find and link OpenCV  
find_package(OpenCV REQUIRED)  
include_directories(${OpenCV_INCLUDE_DIRS})  
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS})  

# Find and link wxWidgets  
find_package(wxWidgets REQUIRED COMPONENTS core base)  
include(${wxWidgets_USE_FILE})  
target_link_libraries(${PROJECT_NAME} ${wxWidgets_LIBRARIES})  

# Installation rules  
install(TARGETS ${PROJECT_NAME} DESTINATION ${INSTALL_BIN_DIR})  
install(FILES facewebcam.xpm DESTINATION ${INSTALL_ICON_DIR})  

# Add a .desktop file for Linux  
if(UNIX AND NOT APPLE)  
    configure_file(${CMAKE_SOURCE_DIR}/FaceWebCam.desktop.in ${CMAKE_BINARY_DIR}/FaceWebCam.desktop @ONLY)  
    install(FILES ${CMAKE_BINARY_DIR}/FaceWebCam.desktop DESTINATION ${INSTALL_DESKTOP_DIR})  
endif()
