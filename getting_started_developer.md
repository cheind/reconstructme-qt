# Introduction #

This page introduces the required steps to build and install this project on your platform.


# Compilation #

**Tools/Framewoks Required**
  * CMake 2.8.5 or higher
  * MS Visual Studio 2010
  * Qt Desktop MSVS10 4.8.{1,2,3}
  * ReconstructMeSDK 1.0.0

**Set up**
  * Checkout code
  * Run CMake GUI and select _source_ folder to GUI and a _build_ folder
    * Press Configure (use native compiler MS Visual Studio 2010 x86)
    * ReconstructMeSDK and Qt will be found by CMake if the Libraries are installed. (It could be that you have to set the path to _Qt bin folder_ manually in the PATH environment variable)
    * Don't matter about google protocol buffers in CMake, since this is shipped in the 3rd\_party folder, and the dependencies are set in CMakeLists.txt. You can see this in the output of the CMake GUI.
    * Press generate.
  * Now, you have a solution called reconstructme-qt.sln.
  * Open the solution.
  * Build the Project ReconstructMeQt.
  * Now you can start ReconstructMeQt.exe

# Generating the Installer #

Once you are all set to compile ReconstructMeQt, you can generate an installer from the current build. This currently requires properietary code from PROFACTOR that is not part of this open source project.

Make sure to have the following tools installed
  * WiX 3.6
  * PROFACTOR robvis CMake.

Then
  * Run CMake GUI
  * Set the correct path of ROBVIS\_CMAKE\_DIRECTORY
  * Configure/Generate. You should see a status message indicating "Installer of WiXInstaller will be placed [...]"
  * Open the resulting project and build WiXInstaller