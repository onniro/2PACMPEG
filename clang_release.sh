#!/bin/bash

WORKDIR="$PWD/release_linux"
SOURCES="$PWD/deps/imgui/imgui*.cpp $PWD/linux_2pacmpeg.cpp"
INCLUDE_DIRS="-I $PWD/deps -I $PWD/deps/imgui -I $PWD/deps/GLFW"
#LINK_LIBS="-static-libstdc++ -lGL -lglfw"
STATIC_LIB_DIR1="/usr/lib/x86_64-linux-gnu"
LINK_LIBS="-lX11 $PWD/deps/GLFW/linux/libglfw3.a -static-libstdc++ -static-libgcc -lGL"
MISC_FLAGS="-O2 -o 2PACMPEG -s"
DEFINES="-D_2PACMPEG_LINUX=1 -D_2PACMPEG_DEBUG=0 -D_2PACMPEG_RELEASE=1"
WARNINGLEVEL="-Wno-parentheses -Wno-format -Wno-unicode-whitespace"

mkdir $WORKDIR
cd $WORKDIR

clang++ $DEFINES $WARNINGLEVEL $INCLUDE_DIRS $OBJ_FILES $SOURCES $LINK_LIBS $MISC_FLAGS

cd ..
