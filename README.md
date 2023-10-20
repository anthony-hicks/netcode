## Objective
Learn about fundamental latency compensation methods that provide clients with a smooth, real-time experience.

The goal is to have a demo program that draws a basic shape on the screen that can be
moved by the player via keyboard input. In the background there will be a client-server
application utilizing latency compensation.

## Roadmap
1. Basic application handling keyboard input and sending real network messages
between a client and server (dumb client)
2. Add latency compensation
3. Add graphics to enhance demonstration

## Requirements
- cross-platform (eventually, linux preferred right now)
- client-side prediction
- entity interpolation

## Future
- replace asio with handwritten modern C++ code 
  - consider using getaddrinfo instead of traditional approach
- use udp with custom reliability and congestion avoidance
- interactive UI to toggle each technique as well as client lag and server update rate

## Setup
### Tools
- SDL2
- C++23
- make
- CMake

### Linux Dependencies (Ubuntu 22.04+)
Don't actually need all of these for this program, but it's what their docs recommend.
```shell
sudo apt-get install \
  build-essential \
  git \
  make \
  pkg-config \
  cmake \
  ninja-build \
  gnome-desktop-testing \
  libasound2-dev \
  libpulse-dev \
  libaudio-dev \
  libjack-dev \
  libsndio-dev \
  libx11-dev \
  libxext-dev \
  libxrandr-dev \
  libxcursor-dev \
  libxfixes-dev \
  libxi-dev \
  libxss-dev \
  libxkbcommon-dev \
  libdrm-dev \
  libgbm-dev \
  libgl1-mesa-dev \
  libgles2-mesa-dev \
  libegl1-mesa-dev \
  libdbus-1-dev \
  libibus-1.0-dev \
  libudev-dev \
  fcitx-libs-dev \
  libpipewire-0.3-dev \
  libwayland-dev \
  libdecor-0-dev
```
