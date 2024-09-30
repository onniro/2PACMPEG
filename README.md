# 2PACMPEG - cool front-end for FFmpeg

<img alt="original icon for 2PACMPEG" src="misc/FFMPAC_OG.ico" style="width:40%;">

## 0. yapping
<p>this program was written with the intention of using it to send
shadowplay clips and whatnot to friends on discord without having
to waste money on nitro.</p>

## 1.1 installation for 2PACMPEG\_WITH\_FFMPEG
- step 1: download the archive where it says "2PACMPEG\_WITH\_FFMPEG" from  <https://github.com/onniro/2PACMPEG/releases>
- step 2: unpack the archive wherever you want

## 1.2 installation for 2PACMPEG\_NO\_FFMPEG
- step 1: obtain copies of the FFmpeg executables (namely ffmpeg and ffprobe)
- step 2: follow the instructions in "installation for 2PACMPEG\_WITH\_FFMPEG", 
        but instead of downloading 2PACMPEG\_WITH\_FFMPEG, download 
        2PACMPEG\_NO\_FFMEPG instead
- step 3: go to the containing folder of the 2PACMPEG executable
- step 4: make a folder there with the name ffmpeg
- step 5: copy (or move) the ffmpeg and ffprobe executables into that folder 

## 2. usage:
- go to the folder where you unpacked the archive and click on where it says 2PACMPEG.EXE
- click on "select input file" to select input file (or type/paste in the path 2 tha file) (**file picker only works on windows**)
- you can also drag and drop files on the window to input the path  
- enter arguments for FFmpeg (you can do this by clicking on one of the presets on the right panel)
- as of 16 September 2024, you can toggle extra options by clicking on                     "toggle argument options" and calculate the required bitrate for a desired output file size.
- to make a new preset, enter arguments into the "args for FFmpeg" -field and click "add to argument presets" to add them to the argument presets
- enter the full path to the file that you want FFmpeg to output (eg. c:\stuff\video.mp4)
- you can also set a default output folder, so that you only have to specify the name of the file, instead of the full path. to do this you have to type in the path to the folder under the field labeled "default output folder" and then click "set as default folder"
note: if you have not set a default path and also only specify the output filename anyway, the resulting file will be output to the containing folder of the ffmpeg executable used by 2PACMPEG
- click on start to launch FFmpeg with currently set arguments

## 3.1 build dependencies 
- clang++ with windows headers and libs
  (e.g <https://github.com/mstorsjo/llvm-mingw>)
- ImGui (included)
- GLFW3 (included)
- (probably could build just fine with other compilers like MSVC but i haven't looked into that)

## 3.2 actually building
- add clang++ to %PATH%/$PATH
- on Windows: cd DRIVE:\dir\to\2pacmpeg && release.bat
- on Linux: cd /dir/to/2pacmpeg && sh release.sh
- (for debug, run compile_imgui.bat/sh **once** and then build with build.bat/sh)

## 4. credits 
- thanks to mastermind graphix designer Karkagami for the icon(s) <3
- GLFW3 - window creation and rendering backend for ImGui
- ImGui - gui toolkit>

