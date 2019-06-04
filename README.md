# oglFusionRealsense
An openGL GLSL implementation of KinectFusion for the Intel Realsense running on the Nvidia Jetson Nano

![Alt text](docs/oglfusionrs.jpg?raw=true "Title")

<h2>Installation</h2>

<h3>Dependencies</h2>


glew glfw3 glm imgui eigen3 realsense2 opencv aruco

sudo apt-get install libglew-dev libglfw3 libglfw3-dev libglm-dev libeigen3-dev

imgui is provided in this repo.

realsense2 was built and installed from source.

opencv was built and installed from source (version 4.1.0).

aruco was built and installed from source (version 1.3.2).

<h3> Installing oglFusion </h3>

We use <a href="https://cmake.org/download/">cmake</a> . Please use the latest version available.

Pull the latest version of oglFusion

```
git clone https://github.com/philipNoonan/oglFusionRealsense

cd oglFusionRealsense

mkdir build && cd build

cmake ..

make -j4

```
Then to run

```
./oglFusionRS

```


<h3> Using oglFusion </h3>

By default, the streams are set up as 848x480 90Hz depth and 90Hz IR, and 1920x1080 6Hz colour. These streams are grabbed using the low level realsense2 api on different threads so they should not cause the main program to lag. 

