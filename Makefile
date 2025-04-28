.PHONY: build test

build:
	mkdir -p build
	cmake --preset=vcpkg
	cmake --build build

test: build
	build/tests
