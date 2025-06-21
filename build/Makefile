CXX = g++
CFLAGS = -std=c++17 -O2
LDFLAGS = -lglfw -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi

TARGET = ../VulkanTest
SRC = ../src/main.cpp ../src/triangle/epic_triangle.cpp
SHADERS = ../shaders/shader.vert ../shaders/shader.frag
SPIRV = ../shaders/vert.spv ../shaders/frag.spv
GLSLC = /usr/bin/glslc

.PHONY: all shaders clean test

all: shaders $(TARGET)

shaders: $(SHADERS)
	$(GLSLC) ../shaders/shader.vert -o ../shaders/vert.spv
	$(GLSLC) ../shaders/shader.frag -o ../shaders/frag.spv

$(TARGET): $(SRC)
	$(CXX) $(CFLAGS) -o $(TARGET) $(SRC) $(LDFLAGS)

test: all
	$(TARGET)

clean:
	rm -f $(TARGET) $(SPIRV)