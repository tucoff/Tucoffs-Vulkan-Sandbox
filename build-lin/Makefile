# ────────────────────────────────────────────────────────────────
# 🛠️  Tucoff's Vulkan Sandbox — Unified Build for All Projects
#
# This Makefile compiles:
#   - main.cpp as a project launcher (text menu)
#   - All C++ source files in src/*/ as independent projects
#   - All shaders (vert/frag) in shaders/*/, compiling them to SPIR-V
#   - Final executable: VulkanSandbox
# ────────────────────────────────────────────────────────────────

# Compiler and shader compiler
CXX      := g++
GLSLC    := /usr/bin/glslc

# Final binary name
TARGET   := ../VulkanSandbox

# ────────────────
# Directory structure
# ────────────────
SRC_DIR     := ../src
SHADER_DIRS := $(wildcard ../shaders/*)

# Source files
MAIN_SRC    := $(SRC_DIR)/main.cpp
UTILS_SRC  := $(SRC_DIR)/utils.cpp
PROJECT_SRCS := $(filter-out $(MAIN_SRC), $(wildcard $(SRC_DIR)/*/*.cpp))

# Include all project folders + root src/
INCLUDES := $(patsubst %,-I%,$(filter %/,$(wildcard $(SRC_DIR)/*/))) -I$(SRC_DIR)

# Build flags
CFLAGS   := -std=c++17 -O2 $(INCLUDES)
LDFLAGS  := -lglfw -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi

# Shader compilation targets
SPIRV := $(foreach dir,$(SHADER_DIRS),$(dir)/shader.vert.spv $(dir)/shader.frag.spv)
VERTS := $(patsubst %.spv,%.vert,$(filter %.vert.spv,$(SPIRV)))
FRAGS := $(patsubst %.spv,%.frag,$(filter %.frag.spv,$(SPIRV)))

.PHONY: all clean shaders test

# ─────────────────────
# Full project compilation
# ─────────────────────
all: shaders $(TARGET)

# Compile all shaders to SPIR-V
shaders: $(SPIRV)

%.vert.spv: %.vert
	$(GLSLC) $< -o $@

%.frag.spv: %.frag
	$(GLSLC) $< -o $@

# Compile main launcher and all project modules
$(TARGET): $(MAIN_SRC) $(UTILS_SRC) $(PROJECT_SRCS)
	$(CXX) $(CFLAGS) $^ -o $@ $(LDFLAGS)

# Build and execute the launcher
test: all
	$(TARGET)

# Clean all generated files
clean:
	rm -f $(TARGET) $(SPIRV)