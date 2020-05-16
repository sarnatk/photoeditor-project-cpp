cmake_minimum_required(VERSION 3.10)
project(photoeditor)


set(CMAKE_CXX_STANDARD 17)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5Widgets REQUIRED)
find_package(Qt5PrintSupport REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Core REQUIRED)
find_package(Qt5Network REQUIRED)
find_package(Qt5Xml REQUIRED)

find_package(OpenCV REQUIRED)

# The Qt5Widgets_INCLUDES also includes the include directories for
# dependencies QtCore and QtGui
include_directories(../ ${Qt5Widgets_INCLUDES} ${Qt5Gui_INCLUDES} ${Qt5Core_INCLUDES} ${Qt5Network_INCLUDES} ${Qt5Xml_INCLUDES} ${Qt5PrintSupport_INCLUDES} ${OpenCV_INCLUDE_DIRS} include)

file(COPY icons DESTINATION .)

# We need add -DQT_WIDGETS_LIB when using QtWidgets in Qt 5.
add_definitions(${Qt5Widgets_DEFINITIONS} ${Qt5Gui_DEFINITIONS} ${Qt5Core_DEFINITIONS} ${Qt5PrintSupport_DEFINITIONS} ${Qt5Network_DEFINITIONS} ${Qt5Xml_DEFINITIONS})

# Executables fail to build with Qt 5 in the default configuration
# without -fPIE. We add that here.
set(CMAKE_CXX_FLAGS "${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS} ${Qt5Core_EXECUTABLE_COMPILE_FLAGS} ${Qt5Gui_EXECUTABLE_COMPILE_FLAGS} ${Qt5PrintSupport_EXECUTABLE_COMPILE_FLAGS} ${Qt5Network_EXECUTABLE_COMPILE_FLAGSS} ${Qt5Xml_EXECUTABLE_COMPILE_FLAGS}")

add_executable(photoeditor include/imageviewer.h include/algorithms.h include/imgur.h src/imageviewer.cpp src/algorithms.cpp src/imgur.cpp src/main.cpp src/controller.cpp include/controller.h)


target_link_libraries(photoeditor ${Qt5Widgets_LIBRARIES} ${Qt5Gui_LIBRARIES} ${Qt5Core_LIBRARIES} ${Qt5PrintSupport_LIBRARIES} ${Qt5Network_LIBRARIES} ${Qt5Xml_LIBRARIES} ${OpenCV_LIBS})
