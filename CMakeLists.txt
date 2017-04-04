cmake_minimum_required(VERSION 2.8.11)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

project(VDSuperVis)

set(VDSV_SRC "${VDSuperVis_SOURCE_DIR}")
set(VTK_DIR "C:/VTK-7.1.0/VTK-bin" CACHE PATH "VTK directory override" FORCE)
set(OpenCV_DIR "D:/OpenCV3/build" CACHE PATH "OpenCV directory override" FORCE)
find_package(VTK 7.1.0 REQUIRED)
find_package(OpenCV REQUIRED)
find_package(Qt5Core REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Gui REQUIRED)

include(${VTK_USE_FILE})
include_directories( ${OpenCV_INCLUDE_DIRS} )

file(GLOB UI_FILES *.ui)
file(GLOB H_FILES *.h)
file(GLOB CPP_FILES *.cpp)

qt5_wrap_ui(UISrcs ${UI_FILES})


add_executable(
  VDSuperVis 

  ${CPP_FILES}
  ${H_FILES}
  ${UISrcs}
  ${WIN_RESOURCE_FILE}
)

target_link_libraries(
  VDSuperVis 
  ${VTK_LIBRARIES}
  ${OpenCV_LIBS}
  Qt5::Widgets
  Qt5::Gui
  Qt5::Core
)

add_custom_command(
  TARGET VDSuperVis POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:Qt5::Widgets>
	$<TARGET_FILE:Qt5::Core>
	$<TARGET_FILE:Qt5::Gui>
	$<TARGET_FILE:${VTK_DIR}/$<CONFIGURATION>
	$<TARGET_FILE_DIR:VDSuperVis>
)