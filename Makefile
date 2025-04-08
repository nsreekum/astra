CXX = g++
CXXFLAGS = -std=c++17 -Wall -Iinclude
LDFLAGS =
BUILD_DIR = build
SRC_DIR = src
TARGET = ar_simulation

SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS = $(SOURCES:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)

# Create build directory if it doesn't exist
$(BUILD_DIR):
	mkdir -p $@

# Default target
all: $(BUILD_DIR) $(TARGET)

# Compile source files into object files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Link object files to create the executable
$(TARGET): $(OBJECTS)
	$(CXX) $(LDFLAGS) $(OBJECTS) -o $@

# Clean up build artifacts
clean:
	rm -rf $(BUILD_DIR) $(TARGET)

# Run the simulation
run: $(TARGET)
	./$(TARGET)