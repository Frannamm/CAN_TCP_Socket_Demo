.PHONY: all build test clean run

# Configure + build everything
all: build

build:
	cmake -S . -B build
	cmake --build build

# Run tests via ctest (concise, shows pass/fail per test)
test: build
	cd build && ctest --output-on-failure

# Run the test binary directly (gtest's own detailed colored output)
run: build
	./build/run_tests

# Wipe the build directory to force a clean rebuild
clean:
	rm -rf build