# oglFusionRealsense
An openGL GLSL implementation of KinectFusion for the Intel Realsense

![Alt text](docs/oglfusionrs.jpg?raw=true "Title")

<h2>Installation</h2>

<h3>Dependencies</h2>

We use vcpkg to install dependencies. Get vcpkg from the link and follow its installation instructions.

<a href="https://github.com/Microsoft/vcpkg">VCPKG</a> 

<h4>Windows</h3>

To make vcpkg use a little cleaner we set two environment variables, defining the tpe of system (x64 / x86) and the location of vcpkg.exe. Open a command promt with administrator privilages (hit windows key, type "cmd", right click "Command Prompt" and choose "Run as Administrator") .
These commands may take a few seconds to execute.

```
setx VCPKG_DEFAULT_TRIPLET "x64-windows" /m
setx VCPKG_ROOT "C:\vcpkg" /m
```
Close the Admin Command Prompt window to flush the newly set variables.

Go to your vcpkg.exe installed location and open another command prompt.

Then we install the various libraries needed for this project.

```
vcpkg install glew glfw3 glm imgui eigen3 realsense2
```
This should take 3-4 minutes.

<h3> Installing oglFusion </h3>

We use <a href="https://www.visualstudio.com/downloads/">visual studio 2017</a> since it is the most readily available MSVC these days, support for c++17 features, and the hope that it will be useable with cuda 9.2.

We use <a href="https://cmake.org/download/">cmake</a> . Please use the latest version available.

Pull the latest version of oglFusion

```
git clone https://github.com/philipNoonan/oglFusionRealsense
```

Open CMake and set the source directory as "PATH_TO_YOUR_VERSION/oglFusionRealsense/" and the build directory as "PATH_TO_YOUR_VERSION/oglFusionRealsense/build"

Choose to create a new folder, and choose MSVC 15 2017 x64 as the generator.

Press "Configure"

Press "Generate"

Press "Open Project"
