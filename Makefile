# Makefile
CXX = g++
CXXFLAGS = -std=c++17 -Wall
TARGET = retrotool
IMGUI_DIR = imgui
SOURCES = main.cpp file_monitor.cpp \
			$(IMGUI_DIR)/imgui.cpp \
			$(IMGUI_DIR)/imgui_draw.cpp \
			$(IMGUI_DIR)/imgui_widgets.cpp \
			$(IMGUI_DIR)/imgui_tables.cpp \
			$(IMGUI_DIR)/backends/imgui_impl_sdl3.cpp \
			$(IMGUI_DIR)/backends/imgui_impl_sdlrenderer3.cpp
OBJECTS = $(SOURCES:.cpp=.o)
HEADERS = main.hpp file_monitor.hpp $(IMGUI_DIR)/imgui.h $(IMGUI_DIR)/backends/imgui_impl_sdl3.h $(IMGUI_DIR)/backends/imgui_impl_sdlrenderer3.h

# SDL3
SDL_CFLAGS = $(shell pkg-config --cflags sdl3)
SDL_LIBS = $(shell pkg-config --libs sdl3)
CXXFLAGS += $(SDL_CFLAGS) -I$(IMGUI_DIR) -I$(IMGUI_DIR)/backends
LIBS = $(SDL_LIBS)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(TARGET) $(LIBS)

%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)

.PHONY: all clean