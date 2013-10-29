cmake_minimum_required(VERSION 2.8)
project( Frogs )
find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )

add_executable( findBlkPxls findBlkPxls.cxx )
target_link_libraries( findBlkPxls ${OpenCV_LIBS} )
