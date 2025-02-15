@echo off
@echo NOTE: make sure to update version macros in 2pacmpeg.h before releasing 4real!

set SOURCES=%CD%\deps\imgui\imgui*.cpp %CD%\win32_2pacmpeg.cpp
REM set SOURCES=%CD%\win32_2pacmpeg.cpp
REM set OBJ_FILES=%CD%\imgui_o_files\*.o
set INCLUDE_DIRS=-I%CD%\deps -I%CD%\deps\imgui -I%CD%\deps\GLFW
set LINK_DIRS=-L %CD%\deps\GLFW\win32
set LINK_LIBS=-static-libstdc++ -static -lunwind -lopengl32 -lgdi32 -lshell32 -lole32 -lglfw3 -lcomctl32 -luuid -lwinmm -lshlwapi
set MISC_FLAGS=-fno-cxx-exceptions -fno-exceptions -target x86_64-pc-windows-gnu -march=x86-64 -O2 -s -o 2PACMPEG.EXE -Wl,/subsystem:windows,/subsystem:console,/opt:ref

set DEFINES=-D _2PACMPEG_WIN32=1 -D _2PACMPEG_RELEASE=1 -D _2PACMPEG_DEBUG=0 -D _NO_DEBUG_HEAP=1 -D_2PACMPEG_ENABLE_CHINESE_SIMPLIFIED=0 -D_2PACMPEG_ENABLE_CHINESE_FULL=0

set WARNINGLEVEL=-Wno-parentheses -Wno-format -Wno-unicode-whitespace

set RES_SCRIPTS=%CD%\2pacmpeg.rc
set RES_FLAGS=-o %CD%\release\2pacmpeg.res
set RES_FILES=%CD%\release\2pacmpeg.res

REM set WORKDIR=-working-directory %CD%\release
set WORKDIR=%CD%\release

mkdir %WORKDIR%
pushd %WORKDIR%

windres %RES_SCRIPTS% %RES_FLAGS%
clang++ %DEFINES% %WARNINGLEVEL% %LINK_DIRS% %INCLUDE_DIRS% %LINK_LIBS% %SOURCES% %RES_FILES% %MISC_FLAGS%
popd

