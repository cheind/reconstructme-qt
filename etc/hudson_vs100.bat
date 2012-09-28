:: @file
:: @copyright Copyright (c) 2012 PROFACTOR GmbH. All rights reserved. 
::
:: Redistribution and use in source and binary forms, with or without
:: modification, are permitted provided that the following conditions are
:: met:
::
::     * Redistributions of source code must retain the above copyright
:: notice, this list of conditions and the following disclaimer.
::     * Redistributions in binary form must reproduce the above
:: copyright notice, this list of conditions and the following disclaimer
:: in the documentation and/or other materials provided with the
:: distribution.
::     * Neither the name of Google Inc. nor the names of its
:: contributors may be used to endorse or promote products derived from
:: this software without specific prior written permission.
::
:: THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
:: "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
:: LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
:: A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
:: OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
:: SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
:: LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
:: DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
:: THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
:: (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
:: OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
::
:: @authors christoph.kopf@profactor.at
::          florian.eckerstorfer@profactor.at

:: ====================================================================
:: This is a build setup file for a hudson build and will only work 
:: in our company. This files enables us to controll the build success
:: of different versions.
:: ====================================================================

:: Make sure to subsitute forward slashes with backslashes. Happens on multi-configuration builds.
set WS=%WORKSPACE:/=\%

:: Set ReMe Version for all projects to be build
set GUI_VERSION_MAJOR=1
set GUI_VERSION_MINOR=0
set GUI_VERSION_BUILD=%BUILD_NUMBER%

rmdir /Q /S %WS%\build

call "%VS100COMNTOOLS%\..\..\VC\bin\vcvars32.bat"

mkdir %WS%\build

:: --------------------------------------------------------------------------------------------
:: Build the non-commercial version
:: --------------------------------------------------------------------------------------------

cd %WS%\build
cmake.exe -G "Visual Studio 10" ^
-DRECONSTRUCTMEQT_VERSION_MAJOR=%GUI_VERSION_MAJOR% ^
-DRECONSTRUCTMEQT_VERSION_MINOR=%GUI_VERSION_MINOR% ^
-DRECONSTRUCTMEQT_VERSION_BUILD=%GUI_VERSION_BUILD% 
%WS%\source\

cd %WS%

"%VS100COMNTOOLS%\..\IDE\devenv.com" "build/reconstructmeqt.sln" /out build.log /build "Release" /project ReconstructMeGUI
if %ERRORLEVEL% NEQ 0 GOTO ERROR

:ERROR
exit %ERRORLEVEL%