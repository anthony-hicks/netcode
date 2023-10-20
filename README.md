## Objective
Learn about fundamental latency compensation methods that provide clients with a smooth, real-time experience.

## TODO
- Should be cross-platform, so need to test on Linux eventually
- dumb client
- client-side prediction (without reconciliation)
- client-side prediction (with reconciliation)
- entity interpolation
- final program
  - uses all techniques
  - interactive UI to toggle each technique, client lag and server update rate

## Linux Dependencies (Ubuntu 22.04+)
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

## Tools
- SDL2
- C++20
- Windows (since WSL2 + Win10 don't have great support for GUI)
- make
- CMake
- VS Code

## Fun, later
- zig as \[cross\]compiler
- compare VS/MSVC to msys2/mingw
- try zig instead of C++
