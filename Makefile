.PHONY: build test

build/gen: CMakeLists.txt vcpkg-configuration.json
	mkdir -p build
	cmake --preset=default -DCMAKE_BUILD_TYPE=Debug
	touch build/gen

build: build/gen
	cmake --build build

test: build
	build/tests
