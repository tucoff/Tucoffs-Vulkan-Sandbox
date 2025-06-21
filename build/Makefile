# build/Makefile

# —————————————————————————————
# Variáveis ajustáveis
# —————————————————————————————
CXX        := g++
GLSLC      := /usr/bin/glslc

# Troque aqui ou por CLI: make PROJECT_ID=2_cube
PROJECT_ID ?= 1_triangle

# —————————————————————————————
# Diretórios e arquivos
# —————————————————————————————
SRC_ROOT    := ../src
SHADER_ROOT := ../shaders

MAIN_SRC    := $(SRC_ROOT)/main.cpp
DEMO_SRCS   := $(wildcard $(SRC_ROOT)/$(PROJECT_ID)/*.cpp)
ALL_SRCS    := $(MAIN_SRC) $(DEMO_SRCS)

VERT_SRC  := $(SHADER_ROOT)/$(PROJECT_ID)/shader.vert
FRAG_SRC  := $(SHADER_ROOT)/$(PROJECT_ID)/shader.frag
VERT_SPV  := $(SHADER_ROOT)/$(PROJECT_ID)/shader.vert.spv
FRAG_SPV  := $(SHADER_ROOT)/$(PROJECT_ID)/shader.frag.spv
ALL_SPV   := $(VERT_SPV) $(FRAG_SPV)

TARGET    := ../VulkanTest_$(PROJECT_ID)

# —————————————————————————————
# Flags de compilação e link
# —————————————————————————————
# Note o -DPROJECT_ID=\"$(PROJECT_ID)\" que injeta o ID no main.cpp
CFLAGS    := -std=c++17 -O2 \
             -DPROJECT_ID=\"$(PROJECT_ID)\" \
             -I$(SRC_ROOT) -I$(SRC_ROOT)/$(PROJECT_ID)
LDFLAGS   := -lglfw -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi

.PHONY: all shaders test clean

# —————————————————————————————
# Alvos
# —————————————————————————————
all: shaders $(TARGET)

shaders: $(ALL_SPV)

$(VERT_SPV): $(VERT_SRC)
	$(GLSLC) $< -o $@

$(FRAG_SPV): $(FRAG_SRC)
	$(GLSLC) $< -o $@

$(TARGET): $(ALL_SRCS)
	$(CXX) $(CFLAGS) $(ALL_SRCS) -o $@ $(LDFLAGS)

test: all
	$(TARGET)

clean:
	rm -f $(TARGET) $(ALL_SPV)