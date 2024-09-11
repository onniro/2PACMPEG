
WORKDIR="$PWD/clang_build_linux"
SOURCES="$PWD/linux_2pacmpeg.cpp"
INCLUDE_DIRS="-I $PWD/deps -I $PWD/deps/imgui -I $PWD/deps/GLFW"
LINK_DIRS=""
OBJ_FILES="$PWD/imgui_o_files_linux/*.o"
LINK_LIBS="-static-libstdc++ -lGL -lglfw"
MISC_FLAGS="-o 2PACMPEG -gdwarf"
DEFINES="-D_2PACMPEG_LINUX=1 -D_2PACMPEG_DEBUG=1 -D_2PACMPEG_RELEASE=0"
WARNINGLEVEL="-Wno-parentheses -Wno-format -Wno-unicode-whitespace"

mkdir $WORKDIR
cd $WORKDIR

clang++ $DEFINES $WARNINGLEVEL $LINK_DIRS $INCLUDE_DIRS $LINK_LIBS $OBJ_FILES $SOURCES $MISC_FLAGS

cd ..
