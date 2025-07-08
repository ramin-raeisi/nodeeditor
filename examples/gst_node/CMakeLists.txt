cmake_minimum_required(VERSION 3.16)

project(gst_pipeline_designer LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Qt6
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Gui)

# Create executable
add_executable(gst_pipeline_designer main.cpp gst_nodes.hpp)

# Link libraries
target_link_libraries(gst_pipeline_designer 
    Qt6::Core
    Qt6::Widgets
    Qt6::Gui
    QtNodes
)

# Include directories
target_include_directories(gst_pipeline_designer PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)