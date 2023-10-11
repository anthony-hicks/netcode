.PHONY: all clean

all: build

clean:
	@rm -rf ./build;

configure:
	@cmake -S . -B build -G Ninja \
	       -DCMAKE_BUILD_TYPE=Debug

build : configure
	@cmake --build build --config Debug
