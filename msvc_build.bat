@echo off

set SOURCES=%CD%\deps\imgui\imgui*.cpp %CD%\win32_2pacmpeg.cpp
set LINK_DIRS=/LIBPATH:%CD%\deps\GLFW
set LINK_LIBS=glfw3.lib opengl32.lib gdi32.lib shell32.lib ole32.lib legacy_stdio_definitions.lib
set LINK_FLAGS=/subsystem:windows
set MISC_FLAGS=/nologo -O2 /Fe:2PACMPEG.EXE /utf-8 /MD
set DEFINES=-D_2PACMPEG_WIN32=1 -D_2PACMPEG_RELEASE=1

mkdir msvc_build
pushd msvc_build

cl %MISC_FLAGS% %DEFINES% %INCLUDE_DIRS% %SOURCES% /link %LINK_DIRS% %LINK_LIBS% %LINK_FLAGS% /MACHINE:x64

popd

