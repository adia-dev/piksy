# Cross-Platform Makefile for Piksy
# Compatible with Linux, macOS, and Windows (MinGW)
# Requires SDL2 and OpenGL

EXE := piksy
EXE_DIR := bin
IMGUI_DIR := imgui
SOURCES_DIR := src
UNAME_S := $(shell uname -s)

# Find source files in src directory and add ImGui sources
SOURCES := $(shell find $(SOURCES_DIR) -type f -name '*.cpp' -not -name '.null-ls*.cpp')
IMGUI_SOURCES := $(IMGUI_DIR)/imgui.cpp $(IMGUI_DIR)/imgui_demo.cpp $(IMGUI_DIR)/imgui_draw.cpp \
                 $(IMGUI_DIR)/imgui_tables.cpp $(IMGUI_DIR)/imgui_widgets.cpp \
                 $(IMGUI_DIR)/backends/imgui_impl_sdl2.cpp $(IMGUI_DIR)/backends/imgui_impl_opengl3.cpp
SOURCES += $(IMGUI_SOURCES)

# Create object file paths based on source files
OBJS := $(patsubst %.cpp, %.o, $(SOURCES))

# Common Compiler Flags
CXX := g++
CXXFLAGS := -std=c++17 -g -Wall -Wformat -I$(IMGUI_DIR) -I$(IMGUI_DIR)/backends -I$(SOURCES_DIR)
LIBS :=

# Platform-Specific Flags and Libraries
ifeq ($(UNAME_S), Linux)
	ECHO_MESSAGE := "Linux"
	LIBS += -lGL -ldl `sdl2-config --libs`
	CXXFLAGS += `sdl2-config --cflags`
endif

ifeq ($(UNAME_S), Darwin)
	ECHO_MESSAGE := "Mac OS X"
	LIBS += -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo `sdl2-config --libs`
	CXXFLAGS += `sdl2-config --cflags` -I/usr/local/include
endif

ifeq ($(OS), Windows_NT)
	ECHO_MESSAGE := "Windows (MinGW)"
	LIBS += -lgdi32 -lopengl32 -limm32 `pkg-config --static --libs sdl2`
	CXXFLAGS += `pkg-config --cflags sdl2`
endif

##---------------------------------------------------------------------
## Build Rules
##---------------------------------------------------------------------

# Default target
all: $(EXE)
	@echo "Build complete for $(ECHO_MESSAGE)"

# Link executable
$(EXE): $(OBJS)
	mkdir -p $(EXE_DIR)
	$(CXX) -o $(EXE_DIR)/$@ $^ $(CXXFLAGS) $(LIBS)

# Compile .cpp to .o
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

run: $(EXE)
	$(EXE_DIR)/$(EXE)

# Clean up build artifacts
clean:
	rm -rf $(EXE_DIR) $(OBJS)

# Generate compile_commands.json for clang-tidy or other tools
bear:
	bear -- make
