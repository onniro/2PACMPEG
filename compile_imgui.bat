@echo off

set WORKDIR=%CD%\imgui_o_files
set SOURCES=%CD%\deps\imgui\imgui*.cpp
set INCLUDE_DIRS=-I..\deps -I..\deps\imgui
set LINK_LIBS=
set MISC_FLAGS=-O3 -c
set DEFINES=-D_NO_DEBUG_HEAP=1

mkdir %WORKDIR%
pushd %WORKDIR%

clang++ %DEFINES% %INCLUDE_DIRS% %LINK_LIBS% %SOURCES% %MISC_FLAGS%

popd

