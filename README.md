## Objective
Learn about fundamental latency compensation methods that provide clients with a smooth, real-time experience.

The goal is to have a demo program that draws a basic shape on the screen that can be
moved by the player via keyboard input. In the background there will be a client-server
application utilizing latency compensation.

## Roadmap
- [x] Basic application handling keyboard input and sending real network messages
between a client and server (dumb client)
- [x] Graphics for player 1 client view of a circle
- [x] Add intentional server delay and/or tick rate to make differences more noticeable
- [x] Client-side prediction
- [x] Server reconciliation
- [x] Entity interpolation
- [ ] Graphics for server view
- [x] Networking for player 2 client, which only receives the position of player 1
- [x] Graphics for player 2 view
- [x] Toggleable GUI for each technique and latency/tick rate
- [x] Add graphics to enhance demonstration
- [x] Add variable timestep and velocity-based position calculation to all programs
- [ ] Add gifs to repo front page
- [x] Handle any number of entities
- [x] Add references

## Requirements
- cross-platform (eventually, linux preferred right now)
- client-side prediction
- entity interpolation

## Future
- refactor and clean up code to be more reusable
- wrap SDL in idiomatic C++
  - utilize some sort of Result<T, E> type to wrap the C layer
    - boost::outcome
    - https://github.com/bitwizeshift/result
    - std::expected

## References
Heavily influenced by https://www.gabrielgambetta.com/client-server-game-architecture.html

## Setup
### Tools
- SDL2
- C++20
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
