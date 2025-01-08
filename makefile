# Compiler and flags
CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 \
    -I/opt/homebrew/opt/python@3.13/Frameworks/Python.framework/Versions/3.13/include/python3.13 \
    -I/opt/homebrew/lib/python3.13/site-packages/pybind11/include

PYTHON = /opt/homebrew/opt/python@3.13/bin/python3.13

# Source files and output binary
SOURCES = main.cpp
OBJECTS = $(SOURCES:.cpp=.o)
TARGET = PasswordManager.exe

# Default rule
all: $(TARGET)

# Linking
$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Compilation
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up
clean:
	rm -f $(OBJECTS) $(TARGET)

# Phony targets
.PHONY: all clean
