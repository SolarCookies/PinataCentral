@echo off

call glslangValidator -V -o basic.vert.spirv basic.vert.glsl
call glslangValidator -V -o basic.frag.spirv basic.frag.glsl

pause