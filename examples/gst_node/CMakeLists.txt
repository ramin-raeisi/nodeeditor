cmake_minimum_required(VERSION 3.16)

project(gst_node LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Qt
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Gui)

# Find GStreamer
find_package(PkgConfig REQUIRED)
pkg_check_modules(GST REQUIRED gstreamer-1.0)

# Create executable
add_executable(gst_node main.cpp gst_nodes.hpp)

# Link libraries
target_link_libraries(gst_node
    Qt6::Core
    Qt6::Widgets
    Qt6::Gui
    QtNodes
    ${GST_LIBRARIES}
)

# Include directories
target_include_directories(gst_node PRIVATE ${GST_INCLUDE_DIRS})
