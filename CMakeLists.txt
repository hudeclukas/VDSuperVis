cmake_minimum_required(VERSION 2.8)

add_definitions(/ZI)

project(VDSuperVis)

set(VDSV_SRC "${VDSuperVis_SOURCE_DIR}")
set(VTK_DIR "C:/VTK-7.1.0/VTK-bin" CACHE PATH "VTK directory override" FORCE)
set(OpenCV_DIR "C:/OpenCV3/build" CACHE PATH "OpenCV directory override" FORCE)
find_package(VTK REQUIRED)
find_package(OpenCV REQUIRED)

include(${VTK_USE_FILE})
include_directories( ${OpenCV_INCLUDE_DIRS} )

set(HEADERTYPEFILES
  Sample.h
  Superpixel.h
)

add_executable(
  VDSuperVis 
  VDSuperVis.cpp
  Sample.cpp
  Superpixel.cpp
  ${HEADERTYPEFILES}
  ${WIN_RESOURCE_FILE}
)

target_link_libraries(
  VDSuperVis 
  ${VTK_LIBRARIES}
  ${OpenCV_LIBS}
)

source_group("Header files" FILES ${SRC_H})

source_group("Source files" FILES ${SRC_S})