CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -g

# Platform-specific libraries
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
    LIBS = -lX11
else ifeq ($(UNAME_S),Darwin)
   
    ifneq ($(wildcard /opt/X11/lib/libX11.dylib),)
        LIBS = -L/opt/X11/lib -lX11
        CXXFLAGS += -I/opt/X11/include
    else
        LIBS = 
    endif
else
    LIBS = 
endif

# Source files
SOURCES = main.cc game.cc player.cc playerFactory.cc cmdInt.cc position.cc piece.cc board.cc display.cc textDisplay.cc graphicalDisplay.cc
OBJECTS = $(SOURCES:.cc=.o)

# Target executables
TARGET = chess
TEST_TARGET = test_players

# Default target
all: $(TARGET)

# Build the main executable
$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJECTS) $(LIBS)



# Compile source files
%.o: %.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean build files
clean:
	rm -f $(OBJECTS) $(TARGET)

# Run the program
run: $(TARGET)
	./$(TARGET)

# Run the test
test: $(TEST_TARGET)
	./$(TEST_TARGET)

# Phony targets
.PHONY: all clean run test 