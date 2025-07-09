# ────────────────────────────────────────────────────────────────
# 🛠️  Tucoff's Vulkan Sandbox — CMake Build for All Projects
#
# Este CMakeLists.txt é uma tentativa de traduzir a lógica do seu Makefile
# para o ambiente CMake, visando a compatibilidade com Visual Studio no Windows.
# ────────────────────────────────────────────────────────────────

cmake_minimum_required(VERSION 3.10)
project(VulkanSandbox CXX) # Define o nome do seu projeto e que ele usa C++

# Definir o diretório raiz do projeto (onde este CMakeLists.txt está)
set(PROJECT_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# ────────────────
# Estrutura de Diretórios
# ────────────────
# As variáveis do seu Makefile indicam que o "Makefile" original estava em um subdiretório
# e 'src' e 'shaders' estavam em '../src' e '../shaders'.
# Se este CMakeLists.txt está na raiz do seu repositório (ao lado de 'src' e 'shaders'),
# então os caminhos serão relativos a esta raiz.

set(SRC_DIR "${PROJECT_ROOT_DIR}/src")
set(SHADER_ROOT_DIR "${PROJECT_ROOT_DIR}/shaders") # Pasta base para os shaders

# ────────────────
# Definições de Pré-Processador
# ────────────────
# Adicionar NOMINMAX para evitar conflito de macros min/max do Windows (MSVC)
add_compile_definitions(NOMINMAX)

# ────────────────
# Arquivos Fonte
# ────────────────
# Seu Makefile indica: main.cpp em src/ e outros .cpp em src/*/
file(GLOB MAIN_CPP_FILE "${SRC_DIR}/main.cpp")
# Isso vai encontrar todos os arquivos .cpp em qualquer subdiretório dentro de 'src' (ex: src/MyProject/file.cpp)
# Se você tiver arquivos .cpp diretamente em 'src/' que não sejam 'main.cpp', esta linha os ignorará.
# Se precisar inclui-los, você pode adicionar um 'file(GLOB PROJECT_ROOT_CPP_FILES "${SRC_DIR}/*.cpp")'
# e adicionar a lista ao ALL_SOURCE_FILES, filtrando main.cpp.
file(GLOB_RECURSE PROJECT_CPP_FILES "${SRC_DIR}/*/*.cpp")

# Combinar todos os arquivos fonte, excluindo o main.cpp dos arquivos de projeto recursivos
set(ALL_SOURCE_FILES "") # Inicializa a lista
list(APPEND ALL_SOURCE_FILES ${MAIN_CPP_FILE})
foreach(proj_file ${PROJECT_CPP_FILES})
    # Adicionar apenas se não for o main.cpp (evita duplicidade caso GLOB_RECURSE o pegue)
    if(NOT "${proj_file}" STREQUAL "${MAIN_CPP_FILE}")
        list(APPEND ALL_SOURCE_FILES ${proj_file})
    endif()
endforeach()

message(STATUS "Found main.cpp: ${MAIN_CPP_FILE}")
message(STATUS "Found project .cpp files: ${PROJECT_CPP_FILES}")
message(STATUS "All source files to compile: ${ALL_SOURCE_FILES}") # Verifique esta saída no console do CMake

# ────────────────
# Criação do Executável Principal
# ────────────────
# ESTE BLOCO DEVE VIR ANTES DE QUALQUER 'target_...' QUE SE REFERE A ELE
add_executable(${PROJECT_NAME} ${ALL_SOURCE_FILES})

# Definir as propriedades do executável (onde o .exe será gerado)
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# ────────────────
# Diretórios de Inclusão (Headers)
# ────────────────
# Seu Makefile inclui 'src/' e todos os seus subdiretórios como includes
set(ALL_INCLUDE_DIRS)
list(APPEND ALL_INCLUDE_DIRS "${SRC_DIR}") # Incluir a pasta src/

# Encontrar todos os subdiretórios dentro de src/ para inclusão (para headers internos)
file(GLOB_RECURSE SRC_SUBDIRS LIST_DIRECTORIES ON "${SRC_DIR}/*")
foreach(dir ${SRC_SUBDIRS})
    if(IS_DIRECTORY "${dir}") # Garantir que é um diretório
        list(APPEND ALL_INCLUDE_DIRS "${dir}")
    endif()
endforeach()

# Adicionar os diretórios de inclusão para o alvo executável
# Usar PRIVATE significa que esses includes são apenas para a compilação deste alvo
target_include_directories(${PROJECT_NAME} PRIVATE
    ${ALL_INCLUDE_DIRS}
)

# ────────────────
# Configurações de Compilação
# ────────────────
# Padrão C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF) # Boa prática para padrões C++

# Sinalizadores de otimização (equivalente a -O2)
# O CMake lida com isso através dos tipos de build (Debug, Release, etc.)
# Para Release (padrao -O2):
# set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2") # Para MSVC
# set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2") # Para GCC/Clang

# ────────────────
# Encontrar Bibliotecas Externas (GLFW, Vulkan)
# ────────────────
# Estas são as linhas críticas para encontrar as bibliotecas no Windows.
# Certifique-se de ter o vcpkg configurado e as bibliotecas instaladas (glfw3, vulkan).

find_package(glfw3 CONFIG REQUIRED) # Procura por GLFW
find_package(Vulkan REQUIRED)       # Procura por Vulkan SDK (inclui glslc)

# ────────────────
# Compilação de Shaders (SPIR-V)
# ────────────────
# Esta é a parte mais complexa para converter.
# Precisamos iterar sobre cada subdiretório em 'shaders/' e compilar os .vert e .frag.

# Encontrar o executável glslc (geralmente vem com o Vulkan SDK)
find_program(GLSLC_EXECUTABLE glslc HINTS "${Vulkan_BINARY_DIR}" ENV PATH)
if(NOT GLSLC_EXECUTABLE)
    message(FATAL_ERROR "glslc not found. Please ensure Vulkan SDK is installed and its bin directory is in PATH or specify its path.")
endif()

# Encontrar os diretórios que contêm os shaders (ex: shaders/basic, shaders/pbr)
file(GLOB SHADER_DIRS LIST_DIRECTORIES ON "${SHADER_ROOT_DIR}/*")

set(ALL_SHADER_SPV_FILES)

foreach(shader_dir ${SHADER_DIRS})
    # Definir caminhos completos para os shaders
    set(VERT_SHADER "${shader_dir}/shader.vert")
    set(FRAG_SHADER "${shader_dir}/shader.frag")

    # Crie o caminho relativo para a subpasta dentro de 'shaders/' (ex: '1_triangle', 'pbr')
    # Isso é usado para manter a estrutura de pasta dos shaders dentro do diretório de build e de destino.
    string(REPLACE "${SHADER_ROOT_DIR}/" "" REL_SHADER_DIR_PATH "${shader_dir}")

    # Definir caminhos de saída para os SPV compilados (dentro do diretório de build)
    # Ex: build-win/bin/1_triangle/shader.vert.spv
    set(OUT_VERT_SPV "${CMAKE_BINARY_DIR}/bin/${REL_SHADER_DIR_PATH}/shader.vert.spv")
    set(OUT_FRAG_SPV "${CMAKE_BINARY_DIR}/bin/${REL_SHADER_DIR_PATH}/shader.frag.spv")

    # Adicionar os SPV gerados à lista de arquivos de shader (para o target 'shaders')
    list(APPEND ALL_SHADER_SPV_FILES ${OUT_VERT_SPV} ${OUT_FRAG_SPV})

    # Adicionar comandos customizados para compilar os shaders
    # Isso criará regras de build para cada shader
    add_custom_command(
        OUTPUT ${OUT_VERT_SPV}
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/bin/${REL_SHADER_DIR_PATH}" # Garante que o diretório de saída exista
        COMMAND ${GLSLC_EXECUTABLE} ${VERT_SHADER} -o ${OUT_VERT_SPV}
        DEPENDS ${VERT_SHADER}
        COMMENT "Compiling ${VERT_SHADER} to SPIR-V"
    )

    add_custom_command(
        OUTPUT ${OUT_FRAG_SPV}
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/bin/${REL_SHADER_DIR_PATH}"
        COMMAND ${GLSLC_EXECUTABLE} ${FRAG_SHADER} -o ${OUT_FRAG_SPV}
        DEPENDS ${FRAG_SHADER}
        COMMENT "Compiling ${FRAG_SHADER} to SPIR-V"
    )

    # ────────────────────────────────────────────────────────────────
    # Pós-Build: Copiar Shaders Compilados (CORREÇÃO DE CAMINHO)
    # ────────────────────────────────────────────────────────────────
    # Copia a pasta COMPILADA de cada shader (ex: build-win/bin/1_triangle/)
    # para a pasta de destino do executável (ex: build-win/bin/Debug/shaders/1_triangle/)
    set(SOURCE_COMPILED_SHADER_DIR "${CMAKE_BINARY_DIR}/bin/${REL_SHADER_DIR_PATH}")
    set(DEST_FINAL_SHADER_DIR "$<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders/${REL_SHADER_DIR_PATH}")

    add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        # Garante que o diretório de destino 'shaders/1_triangle' (etc.) exista
        COMMAND ${CMAKE_COMMAND} -E make_directory ${DEST_FINAL_SHADER_DIR}
        # Copia o conteúdo da pasta compilada para o destino final
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${SOURCE_COMPILED_SHADER_DIR} ${DEST_FINAL_SHADER_DIR}
        DEPENDS ${OUT_VERT_SPV} ${OUT_FRAG_SPV} # Garante que os shaders estejam compilados antes de copiar
        COMMENT "Copying compiled shader directory ${REL_SHADER_DIR_PATH} to executable directory"
    )

endforeach()

# Adicionar um alvo "shaders" que depende de todos os arquivos SPV
# Isso garante que os shaders sejam compilados como parte do processo de build 'all'
add_custom_target(shaders ALL DEPENDS ${ALL_SHADER_SPV_FILES})

# ────────────────
# Vinculação de Bibliotecas
# ────────────────
target_link_libraries(${PROJECT_NAME} PRIVATE
    glfw # Nome do target/biblioteca fornecido pelo find_package(glfw3)
    ${Vulkan_LIBRARIES} # Variável fornecida pelo find_package(Vulkan)
)