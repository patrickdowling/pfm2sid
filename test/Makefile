BUILD_DIR ?= ./build

.PHONY: build
build:
	@ninja -C $(BUILD_DIR)

.PHONY: test
test:
	@ninja -C $(BUILD_DIR) test

.PHONY: wrap
wrap:
	mkdir -p ./subprojects
	meson wrap install gtest
	meson wrap install fmt

.PHONY: setup
setup:
	meson setup $(BUILD_DIR) -Dwerror=true

.PHONY: clean
clean:
	@ninja -C $(BUILD_DIR) clean
