cmake_minimum_required(VERSION 2.8.9)
project(tracking)
set(CMAKE_BUILD_TYPE Debug)

#Sub Directories
#STRING(TOLOWER ${PROJECT_NAME} LIBRARY_NAME)
#ADD_SUBDIRECTORY(${LIBRARY_NAME})
#ADD_SUBDIRECTORY(examples)

############################################################
## Required Dependencies
#find_package(Boost COMPONENTS thread filesystem system REQUIRED QUIET)
#find_package(Eigen3 REQUIRED)
#find_package(GLUT REQUIRED)
find_package(OpenCV REQUIRED)
#find_package(Pangolin REQUIRED)

# Apply required dependencies settings
#LIST(APPEND USER_INC "${Boost_INCLUDE_DIRS};${Pangolin_INCLUDE_DIRS};${CMAKE_CURRENT_SOURCE_DIR};#${CMAKE_CURRENT_BINARY_DIR};${EIGEN3_INCLUDE_DIR};${OpenCV_INCLUDE_DIRS}")
#LIST(APPEND LIB_DIRS "${Boost_LIBRARY_DIRS};${Pangolin_LIBRARY_DIRS};${PROJECT_BINARY_DIR}/src;${OpenCV_LIB_DIR}")
#LIST(APPEND LINK_LIBS "${Boost_LIBRARIES};${GLUT_LIBRARIES};${Pangolin_LIBRARIES};${OpenCV_LIBS}")

############################################################
## Add Libraries / Include Directories / Link Directories
include_directories(
	${CMAKE_SOURCE_DIR}/include
	${OpenCV_INCLUDE_DIRS})
link_directories(${OpenCV_LIB_DIR})
link_libraries(${OpenCV_LIBS})

#Set Source File
file(GLOB SOURCES "src/*.cpp" "include/*.h")
 
#Executable
add_executable(${PROJECT_NAME} ${SOURCES})

#Generate the shared library from the sources
#add_library(template SHARED ${SOURCES})

#Generate the static library from the sources
#add_library(template STATIC ${SOURCES})
 
#Set the location for library installation -- i.e., /usr/lib in this case
# not really necessary in this example. Use "sudo make install" to apply
#install(TARGETS template DESTINATION /usr/lib)


