
WORKDIR=$PWD/imgui_o_files_linux
SOURCES=$PWD/deps/imgui/imgui*.cpp
INCLUDE_DIRS=-I$PWD/deps -I$PWD/deps/imgui
MISC_FLAGS=-c
DEFINES=-D_NO_DEBUG_HEAP=1

mkdir $WORKDIR
cd $WORKDIR

clang++ $DEFINES $INCLUDE_DIRS $SOURCES $MISC_FLAGS

cd ..


