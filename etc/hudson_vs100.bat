
:: Make sure to subsitute forward slashes with backslashes. Happens on multi-configuration builds.
set WS=%WORKSPACE:/=\%

:: Set ReMe Version for all projects to be build
set GUI_VERSION_MAJOR=1
set GUI_VERSION_MINOR=1
set GUI_VERSION_BUILD=%BUILD_NUMBER%

rmdir /Q /S %WS%\build

call "%VS100COMNTOOLS%\..\..\VC\bin\vcvars32.bat"

mkdir %WS%\build

cd %WS%\build
cmake.exe -G "Visual Studio 10" ^
-DRECONSTRUCTMEQT_VERSION_MAJOR=%GUI_VERSION_MAJOR% ^
-DRECONSTRUCTMEQT_VERSION_MINOR=%GUI_VERSION_MINOR% ^
-DRECONSTRUCTMEQT_VERSION_BUILD=%GUI_VERSION_BUILD% ^
-DROBVIS_CMAKE_DIRECTORY=%WS%/source_cmake/ ^
%WS%\source\

cd %WS%

"%VS100COMNTOOLS%\..\IDE\devenv.com" "build/reconstructme-qt.sln" /out build.log /build "Release" /project ReconstructMeQtBootstrapper
if %ERRORLEVEL% NEQ 0 GOTO ERROR

:ERROR
exit %ERRORLEVEL%