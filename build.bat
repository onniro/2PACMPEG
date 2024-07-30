@echo off

set WORKDIR=%CD%\clang_build
REM set SOURCES=%CD%\deps\imgui\imgui*.cpp %CD%\win32_2pacmpeg.cpp
set SOURCES=%CD%\win32_2pacmpeg.cpp
set INCLUDE_DIRS=-I..\deps -I..\deps\imgui -I..\deps\glfw
set LINK_DIRS=-L ..\deps\GLFW
set OBJ_FILES=%CD%\imgui_o_files\*.o
REM set LINK_LIBS=-static-libstdc++ -static -lunwind -lopengl32 -lgdi32 -lshell32 -lole32 -lglfw3 -luuid -lwinmm -lshlwapi
set LINK_LIBS=-static-libstdc++ -static -lunwind -lopengl32 -lgdi32 -lshell32 -lole32 -lglfw3 -luuid -lwinmm -lshlwapi %OBJ_FILES%
set MISC_FLAGS=-o 2PACMPEG.EXE -g -gcodeview --for-linker --pdb=2pacmpeg.pdb -Wl,/subsystem:windows,/opt:ref
REM set MISC_FLAGS=-o 2PACMPEG.EXE -gdwarf -Wl,/subsystem:windows,/opt:ref
set DEFINES=-D_2PACMPEG_WIN32=1 -D_2PACMPEG_DEBUG=1 -D_2PACMPEG_RELEASE=0 -D_NO_DEBUG_HEAP=1
set WARNINGLEVEL=-Wno-parentheses -Wno-format -Wno-unicode-whitespace

mkdir %WORKDIR%
pushd %WORKDIR%

clang++ %DEFINES% %WARNINGLEVEL% %LINK_DIRS% %INCLUDE_DIRS% %LINK_LIBS% %SOURCES% %MISC_FLAGS%

popd

