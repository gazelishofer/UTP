.PHONY: build run clean help all

BUILD_DIR := cmake-build-debug
EXECUTABLE := $(BUILD_DIR)/UTP

help:
	@echo "UTP Student Management System - Makefile"
	@echo ""
	@echo "Available targets:"
	@echo "  make build       - Build the project"
	@echo "  make run         - Run the executable (requires build)"
	@echo "  make all         - Build and run (default)"
	@echo "  make clean       - Clean build directory"
	@echo "  make rebuild     - Clean and build"
	@echo ""

build:
	@echo "Building project..."
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && cmake .. && cmake --build .
	@echo "Build complete!"

run: $(EXECUTABLE)
	@echo "Running UTP..."
	@./$(EXECUTABLE)

all: build run

rebuild: clean build

clean:
	@echo "Cleaning build directory..."
	@rm -rf $(BUILD_DIR)
	@echo "Clean complete!"

$(EXECUTABLE): build
	@true
