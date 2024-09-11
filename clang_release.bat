@echo off

set SOURCES=%CD%\deps\imgui\imgui*.cpp %CD%\win32_2pacmpeg.cpp
set INCLUDE_DIRS=-I%CD%\deps -I%CD%\deps\imgui -I%CD%\deps\glfw
set LINK_DIRS=-L %CD%\deps\GLFW
set LINK_LIBS=-static-libstdc++ -static -lunwind -lopengl32 -lgdi32 -lshell32 -lole32 -lglfw3 -lcomctl32 -luuid -lwinmm -lshlwapi
set MISC_FLAGS=-fno-cxx-exceptions -fno-exceptions -target x86_64-pc-windows-gnu -march=x86-64 -O2 -s -o 2PACMPEG.EXE -Wl,/subsystem:windows,/opt:ref
set DEFINES=-D_2PACMPEG_WIN32=1 -D_2PACMPEG_RELEASE=1 -D_2PACMPEG_DEBUG=0 -D_NO_DEBUG_HEAP=1
set WARNINGLEVEL=-Wno-parentheses -Wno-format -Wno-unicode-whitespace

set RES_SCRIPTS=%CD%\2pacmpeg.rc
set RES_FLAGS=-o %CD%\release\2pacmpeg.res
set RES_FILES=%CD%\release\2pacmpeg.res

set WORKDIR=-working-directory %CD%\release

mkdir release

windres %RES_SCRIPTS% %RES_FLAGS%
clang++ %WORKDIR% %DEFINES% %WARNINGLEVEL% %LINK_DIRS% %INCLUDE_DIRS% %LINK_LIBS% %SOURCES% %RES_FILES% %MISC_FLAGS%

