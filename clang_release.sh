#!/bin/bash

WORKDIR="$PWD/release_linux"
SOURCES="$PWD/deps/imgui/imgui*.cpp $PWD/linux_2pacmpeg.cpp"
INCLUDE_DIRS="-I $PWD/deps -I $PWD/deps/imgui -I $PWD/deps/GLFW"
#LINK_DIRS=""
#OBJ_FILES="$PWD/imgui_o_files_linux/*.o"
LINK_LIBS="-static-libstdc++ -lGL -lglfw"
MISC_FLAGS="-O2 -o 2PACMPEG -s"
DEFINES="-D_2PACMPEG_LINUX=1 -D_2PACMPEG_DEBUG=0 -D_2PACMPEG_RELEASE=1"
WARNINGLEVEL="-Wno-parentheses -Wno-format -Wno-unicode-whitespace"

mkdir $WORKDIR
cd $WORKDIR

clang++ $DEFINES $WARNINGLEVEL $LINK_DIRS $INCLUDE_DIRS $LINK_LIBS $OBJ_FILES $SOURCES $MISC_FLAGS

cd ..
