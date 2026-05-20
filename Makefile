CXX      = g++
CXXFLAGS = -std=c++17 -g -O2
SRC      = main.cpp shader.cpp
TARGET   = golf

# WSL / Linux — install deps with:
#   sudo apt-get install libglfw3-dev libglew-dev libglm-dev

INCDIRS  = $(shell pkg-config --cflags glfw3 2>/dev/null)
LIBS     = -lglfw -lGLEW -lGL -ldl -pthread

$(TARGET): $(SRC) golf_vert.glsl golf_frag.glsl sky_vert.glsl sky_frag.glsl shader.hpp
	$(CXX) $(CXXFLAGS) $(INCDIRS) $(SRC) $(LIBS) -o $(TARGET)

clean:
	rm -f $(TARGET)
