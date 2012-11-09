# Make sure required functions are included
include(${PATHS_CMAKE_DIR}/functions/target_creator.cmake)

# Set some convenience variables
set(BINARY_DIRECTORY ${PATHS_CMAKE_3RDPARTY_DIR}/osg-3.0.1/bin)
set(INCLUDE_DIRECTORY ${PATHS_CMAKE_3RDPARTY_DIR}/osg-3.0.1/inc)

# Import libraries
create_imported_shared_library(
OpenThreads
${BINARY_DIRECTORY}/ot12-OpenThreadsd.dll
${BINARY_DIRECTORY}/OpenThreadsd.lib
${BINARY_DIRECTORY}/ot12-OpenThreads.dll
${BINARY_DIRECTORY}/OpenThreads.lib
)

create_imported_shared_library(
osg
${BINARY_DIRECTORY}/osg80-osgd.dll
${BINARY_DIRECTORY}/osgd.lib
${BINARY_DIRECTORY}/osg80-osg.dll
${BINARY_DIRECTORY}/osg.lib
)

create_imported_shared_library(
osgGA
${BINARY_DIRECTORY}/osg80-osgGAd.dll
${BINARY_DIRECTORY}/osgGAd.lib
${BINARY_DIRECTORY}/osg80-osgGA.dll
${BINARY_DIRECTORY}/osgGA.lib
)

create_imported_shared_library(
osgViewer
${BINARY_DIRECTORY}/osg80-osgViewerd.dll
${BINARY_DIRECTORY}/osgViewerd.lib
${BINARY_DIRECTORY}/osg80-osgViewer.dll
${BINARY_DIRECTORY}/osgViewer.lib
)

create_imported_shared_library(
osgDB
${BINARY_DIRECTORY}/osg80-osgDBd.dll
${BINARY_DIRECTORY}/osgDBd.lib
${BINARY_DIRECTORY}/osg80-osgDB.dll
${BINARY_DIRECTORY}/osgDB.lib
)

create_imported_shared_library(
osgUtil
${BINARY_DIRECTORY}/osg80-osgUtild.dll
${BINARY_DIRECTORY}/osgUtild.lib
${BINARY_DIRECTORY}/osg80-osgUtil.dll
${BINARY_DIRECTORY}/osgUtil.lib
)

create_imported_shared_library(
osgSim
${BINARY_DIRECTORY}/osg80-osgSimd.dll
${BINARY_DIRECTORY}/osgSimd.lib
${BINARY_DIRECTORY}/osg80-osgSim.dll
${BINARY_DIRECTORY}/osgSim.lib
)

create_imported_shared_library(
osgAnimation
${BINARY_DIRECTORY}/osg80-osgAnimationd.dll
${BINARY_DIRECTORY}/osgAnimationd.lib
${BINARY_DIRECTORY}/osg80-osgAnimation.dll
${BINARY_DIRECTORY}/osgAnimation.lib
)

create_imported_shared_library(
osgQt
${BINARY_DIRECTORY}/osg80-osgQtd.dll
${BINARY_DIRECTORY}/osgQtd.lib
${BINARY_DIRECTORY}/osg80-osgQt.dll
${BINARY_DIRECTORY}/osgQt.lib
)


# Set the all targets variable to contail all imported libraries
set(OSG_ALL_TARGETS OpenThreads osg osgGA osgViewer osgDB osgUtil osgSim osgAnimation osgQt)