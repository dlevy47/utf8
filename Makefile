BUILD_DIR ?= .clangd-build
PRESET ?= nix
BUILD_TYPE ?= Debug
LLDB ?= lldb
DEBUGSERVER ?= /Applications/Xcode.app/Contents/SharedFrameworks/LLDB.framework/Versions/A/Resources/debugserver

.PHONY: configure build test debug clean

configure:
	cmake --preset=$(PRESET) -DCMAKE_BUILD_TYPE=$(BUILD_TYPE)

build: configure
	cmake --build $(BUILD_DIR)

test: build
	ctest --test-dir $(BUILD_DIR) --output-on-failure

debug: build
	LLDB_DEBUGSERVER_PATH=$(DEBUGSERVER) $(LLDB) $(BUILD_DIR)/tests

clean:
	rm -rf build/
