# 2PACMPEG - graphical FFmpeg front-end for boss playas

<img alt="original icon for 2PACMPEG" src="misc/FFMPAC_OG.ico" style="width:40%;">

_pac was tha meanin of lyfe..._

# 0. yapping
<p>this program was written for the purpose of performing certain processing (such as compression or format conversion) on shadowplay clips and whatnot so that they can be uploaded to discord without having to increase the file upload size limit by wasting money on nitro</p>

# 1. installation
## 1.1. Windows
- step 1: obtain copies of ffmpeg.exe and ffprobe.exe (<https://www.ffmpeg.org/download.html>)
- step 2: download the archive where it says something like 2PACMPEG_WINDOWS_x64 from <https://github.com/onniro/2PACMPEG/releases>
- step 3: unpack the archive somewhere
- step 4: go to the containing folder of 2PACMPEG.EXE
- step 5: make a folder there with the name ffmpeg
- step 6: put ffmpeg.exe and ffprobe.exe into the aforementioned folder

## 1.2. Linux
- step 1: get ffmpeg and ffprobe from your package manager
- step 2: see section 3 lol

## 2. usage:
- go to the folder where you unpacked the archive and click on the thing where it says 2PACMPEG
- click on "select input file" to select input file (or type/paste in the path 2 tha file) (file picker only exists on windows)
- you can also drag and drop a file on the window to input the path  
- enter arguments for FFmpeg (you can do this by clicking on one of the presets on the right panel)
- you can also toggle extra options by clicking on "toggle argument options" and calculate the required bitrate for a desired output file size. (NOTE: this only takes video bitrate into account, so you may need to tweak audio bitrate with the value after the -b:a option if the output file size keeps exceeding the desired size)
- to make a new preset, enter arguments into the "args for FFmpeg" -field and click "add to argument presets" to add them to the argument presets
- enter the full path to the file that you want FFmpeg to output (eg. C:\files\file.mp4)
- if you would like to always output to the same folder, without having to specify the full path every time, you can type the path to the desired folder in the "default output folder" -field and click on "set as default folder" (NOTE: you don't have to click this, but doing so saves this setting). after doing this you can always only specify the name of the file (e.g. "file.mp4" instead of "C:\files\file.mp4")(NOTE: if you have not set a default path and also only specify the output file name anyway, the file will be output to the containing folder of the ffmpeg executable used by 2PACMPEG)
- click on "start FFmpeg" to launch FFmpeg with currently set arguments

# 3. building

## 3.1. tools
- clang++ (with windows-specific headers and libs included if on windows)

## 3.2. dependencies 
- ImGui (included)
- GLFW3 (included)

## 3.3. actually building
- append containing folder of clang++ to PATH shell environment variable if it's not already there
- on windows, run clang_release.bat in its containing folder and clang_release.sh on linux
- (for debug, run compile_imgui.bat/sh **once** and then build with build.bat/sh)

# 4. credits 
- thanks to mastermind graphix designer Karkagami for the icon(s) <3
- GLFW3 - window creation and rendering backend for ImGui
- ImGui - gui toolkit
- (most importantly) FFmpeg 

