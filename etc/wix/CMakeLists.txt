# @file
# @copyright Copyright (c) 2012 PROFACTOR GmbH. All rights reserved. 
# 
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
# 
#     * Redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above
# copyright notice, this list of conditions and the following disclaimer
# in the documentation and/or other materials provided with the
# distribution.
#     * Neither the name of Google Inc. nor the names of its
# contributors may be used to endorse or promote products derived from
# this software without specific prior written permission.
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
# 
# @authors Christoph Heindl
# 

# To generate the installer we currently use proprietary CMake functionality
# by PROFACTOR which is currently not part of the open-source distribution.

# Check for required PROFACTOR CMake functionality
set(ROBVIS_CMAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/robvis/cmake CACHE PATH "Directory that contains the robvis cmake directory.")
if (NOT IS_DIRECTORY ${ROBVIS_CMAKE_DIRECTORY})
	message(STATUS "Required PROFACTOR CMake was not found. It is required for installer. Skipping installer")
	return()
endif ()
include(${ROBVIS_CMAKE_DIRECTORY}/functions/util_creators.cmake)

# Retrieve a path from an environment variable and fix backslash problems
macro(get_env_path var name)
   set(${var} $ENV{${name}})
   string( REGEX REPLACE "\\\\" "/" ${var} ${${var}} )
endmacro(get_env_path var name)

# Create installer target
robvis_create_wix_installer(WiXInstaller)
add_dependencies(WiXInstaller ReconstructMeQt)

# Collect necessary ReconstructMe SDK binaries
set(HEAT_OPTS -dr INSTALLDIR 
			  -cg ReconstructMeSDKRuntimeFiles 
			  -var var.SDKRuntimeFilesDirectory
			  -gg -scom -sreg -sfrag -srd)
			  
get_env_path(REME_ROOT_FIXED REME_ROOT_DIRECTORY)
	
robvis_wix_installer_heat_directory(
	WiXInstaller 
	"${REME_ROOT_FIXED}/bin"
	CANDLE_SOURCES
	HEAT_OPTS)

# Compile relevant files	
set(CANDLE_OPTS -dSDKRuntimeFilesDirectory="${REME_ROOT_FIXED}/bin")
GET_TARGET_PROPERTY(RECONSTRUCTMEQT_EXE ReconstructMeQt RELEASE_LOCATION)
set(CANDLE_SOURCES ${CANDLE_SOURCES} Main.wxs QtRuntimes.wxs ReconstructMeQtRuntimes.wxs)

robvis_wix_installer_compile(
	WiXInstaller
	CANDLE_SOURCES
	CANDLE_OBJS
	CANDLE_OPTS)
	
# Generate final .msi
set(LIGHT_OPTS -ext WixUIExtension)
robvis_wix_installer_link(
	WiXInstaller
	CANDLE_OBJS
	"${CMAKE_CURRENT_BINARY_DIR}/ReconstructMeQt-${RECONSTRUCTMEQT_VERSION_MAJOR}.${RECONSTRUCTMEQT_VERSION_MINOR}.${RECONSTRUCTMEQT_VERSION_BUILD}.msi"
	LIGHT_OPTS)
	
	




	
