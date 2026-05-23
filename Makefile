CXX      = g++
<<<<<<< HEAD
CXXFLAGS = -std=c++17 -g -O2
SRC      = main.cpp shader.cpp
TARGET   = golf

# WSL / Linux — install deps with:
#   sudo apt-get install libglfw3-dev libglew-dev libglm-dev

INCDIRS  = $(shell pkg-config --cflags glfw3 2>/dev/null)
LIBS     = -lglfw -lGLEW -lGL -ldl -pthread
=======
CXXFLAGS = -std=c++11 -g -O2
SRC      = main.cpp shader.cpp
TARGET   = golf

# Detect platform
UNAME := $(shell uname)

ifeq ($(UNAME), Darwin)
  # macOS — install deps with: brew install glfw glew glm
  BREW     := $(shell brew --prefix 2>/dev/null || echo /usr/local)
  INCDIRS  = -I$(BREW)/include
  LIBS     = -L$(BREW)/lib -lglfw -lGLEW \
             -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo
else
  # WSL / Linux — install deps with:
  #   sudo apt-get install libglfw3-dev libglew-dev libglm-dev
  INCDIRS  = $(shell pkg-config --cflags glfw3 2>/dev/null)
  LIBS     = -lglfw -lGLEW -lGL -ldl -pthread
endif
>>>>>>> 1ec622464d8af3aea218c363d4bd521a3e6a13b7

$(TARGET): $(SRC) golf_vert.glsl golf_frag.glsl sky_vert.glsl sky_frag.glsl shader.hpp
	$(CXX) $(CXXFLAGS) $(INCDIRS) $(SRC) $(LIBS) -o $(TARGET)

clean:
	rm -f $(TARGET)
