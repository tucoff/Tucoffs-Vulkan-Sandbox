#!/bin/bash
GLSLC=/usr/bin/glslc
$GLSLC shaders/1_triangle/shader.vert -o shaders/1_triangle/vert.spv
$GLSLC shaders/1_triangle/shader.frag -o shaders/1_triangle/frag.spv