#!/bin/bash

WORKDIR="$PWD/clang_build_linux"
SOURCES="$PWD/linux_2pacmpeg.cpp"
INCLUDE_DIRS="-I $PWD/deps -I $PWD/deps/imgui -I $PWD/deps/GLFW"
LINK_DIRS=""
OBJ_FILES="$PWD/imgui_o_files_linux/*.o"
#LINK_LIBS="-static-libstdc++ -lX11 $PWD/deps/GLFW/linux/libglfw3.a -lGL"
LINK_LIBS="-static-libstdc++ -lglfw -lGL -lfontconfig"
MISC_FLAGS="-O0 -o 2pacmpeg -gdwarf -finput-charset=UTF-8 -fexec-charset=UTF-8"
DEFINES="-D_2PACMPEG_LINUX=1 -D_2PACMPEG_DEBUG=1 -D_2PACMPEG_RELEASE=0 -D_2PACMPEG_ENABLE_CHINESE_SIMPLIFIED=0 -D_2PACMPEG_ENABLE_CHINESE_FULL=0" 
WARNINGLEVEL="-Wno-parentheses -Wno-format -Wno-unicode-whitespace"

mkdir $WORKDIR
cd $WORKDIR

clang++ $DEFINES $WARNINGLEVEL $LINK_DIRS $INCLUDE_DIRS $SOURCES $OBJ_FILES $LINK_LIBS $MISC_FLAGS

cd ..
