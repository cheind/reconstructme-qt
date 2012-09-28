
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
-DRECONSTRUCTMEQT_VERISON_MAJOR=%GUI_VERSION_MAJOR% ^
-DRECONSTRUCTMEQT_VERISON_MINOR=%GUI_VERSION_MINOR% ^
-DRECONSTRUCTMEQT_VERISON_BUILD=%GUI_VERSION_BUILD% ^
%WS%\source\

cd %WS%

"%VS100COMNTOOLS%\..\IDE\devenv.com" "build/reconstructmeqt.sln" /out build.log /build "Release" /project ReconstructMeGUI
if %ERRORLEVEL% NEQ 0 GOTO ERROR

:ERROR
exit %ERRORLEVEL%