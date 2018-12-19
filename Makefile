
.PHONY: default
default: all

build/build.ninja:
	mkdir -p build
	meson build

all: build/build.ninja
	ninja -C build

clean: build/build.ninja
	ninja -C build clean

reconfigure: build/build.ninja
	ninja -C build reconfigure

install: build/build.ninja
	ninja -C build install
