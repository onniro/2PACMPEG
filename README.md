# 2PACMPEG - graphical FFmpeg front-end for doctors and nuclear scientists

<img alt="original icon for 2PACMPEG" src="misc/FFMPAC_OG.ico" style="width:40%;">

_pac wuz tha meanin of lyfe..._


### contents:
[0. no one cares](#no-one-cares)

[1. installation](#installation)  
 - [1.1 Windows ](#installation-windows)
 - [1.2 Linux ](#installation-linux)

[2. usage](#usage)  
 - [2.1 2PACMPEG](#usage-2pacmpeg)  
 - [2.2 version 3 stuff (2PACDLP & other ~~bloat~~ useful changes)](#usage-ver3)  

[3. building](#building)  
 - [3.1 tools](#building-tools)  
 - [3.2 dependencies](#building-dependencies)  
 - [3.3 actually building](#building-fr)  

[4. credits](#credits)  

# 0. no one cares <a name="no-one-cares"></a>
<p>this program was written for the purpose of making it slightly easier to do certain processing (such as compression and format conversion) specifically on shadowplay clips and whatnot so that they can be uploaded to discord without having to increase the file upload size limit by wasting money on nitro. the author of this program is unemployed</p>

# 1. installation <a name="installation"></a>
## 1.1. Windows <a name="installation-windows"></a>
- step 1: get copies of ffmpeg.exe and ffprobe.exe (<https://www.ffmpeg.org/download.html>)
- step 2: download 2PACMPEG.EXE from <https://github.com/onniro/2PACMPEG/releases>
- step 3: go to the containing folder of 2PACMPEG.EXE
- step 4: make a folder there with the name ffmpeg
- step 5: put ffmpeg.exe (and ffprobe.exe) into that folder
- a valid 2PACMPEG installation looks something like this:  
```
C:\EXAMPLE 2PACMPEG FOLDER\   
├── 2PACMPEG.EXE  
├── PRESETFILE  
└── ffmpeg\  
   ├── ffmpeg.exe  
   ├── ffprobe.exe (optional)  
   └── yt-dlp.exe (optional)  
```

## 1.2. Linux <a name="installation-linux"></a>
- step 1: get ffmpeg and ffprobe from your package manager
- step 2: see section 3 lol

## 2. usage <a name="usage"></a>
## 2.1 2PACMPEG <a name="usage-2pacmpeg"></a>
- go to the folder where you put 2PACMPEG.EXE and click on it
- click on "select input file" to select input file (or type/paste in the path 2 tha file) (file picker only exists on windows)
- you can also drag and drop a file on the window to input the path  
- enter arguments for FFmpeg (you can do this by clicking on one of the presets on the right panel)
- you can also toggle extra options by clicking on "toggle argument options" and calculate the required bitrate for a desired output file size. (NOTE: this only takes video bitrate into account, so you may need to tweak audio bitrate with the value after the -b:a option if the output file size keeps exceeding the desired size)
- to make a new preset, enter arguments into the "args for FFmpeg" -field and click "add to argument presets" to add them to the argument presets
- enter the full path to the file that you want FFmpeg to output (eg. C:\files\file.mp4)
- (**version 1 and 2 specific, read about version 3 stuff below**) if you would like to always output to the same folder, without having to specify the full path every time, you can type the path to the desired folder in the "default output folder" -field and click on "set as default folder" (NOTE: you don't have to click this, but doing so saves this setting). after doing this you can always only specify the name of the file (e.g. "file.mp4" instead of "C:\files\file.mp4")(NOTE: if you have not set a default path and also only specify the output file name anyway, the file will be output to the containing folder of the ffmpeg executable used by 2PACMPEG)
- click on "start FFmpeg" to launch FFmpeg with currently set arguments
- ffmpeg manual: <https://ffmpeg.org/ffmpeg.html>

## 2.2 version 3 stuff (2PACDLP & other ~~bloat~~ useful changes) <a name="usage-ver3"></a>
- in order to satisfy the need to add more shit that nobody needs, 2PACMPEG v3 has added 2 main changes, the former of which is 2PACDLP; a front end for yt-dlp
- to start using 2PACDLP, all you need to do is get a copy of yt-dlp.exe from <https://github.com/yt-dlp/yt-dlp/releases> and put it in the same folder as your ffmpeg executable
- linux users can just download it from their package manager or whatever as long as its containing directory is in PATH
- in order to toggle between 2PACMPEG and 2PACDLP, click the green text in the top left corner
- for [colorblind or otherwise mentally challenged users](https://github.com/FerBlazt) who may have trouble finding the green text in the top left we have generously added a keyboard shortcut for this which is CTRL+TAB
- to download stuff, input the url in the "url to download" input box and enter the output path in the "output path" box and then click download
- you can fill out additional options, which are primitive for now, such as disable audio/video and which output format you want
- note: advanced users of yt-dlp may enter additional command line options into the url box before the url
- the other change in v3 is something that also affects 2PACMPEG, which is the ability to save multiple common output folders
- in order to use this feature, input the folder you want 2PACMPEG to remember and click "save"
- you can then input one of these folders by clicking on the downward arrow icon before the input box
- these folders are saved in a file called 2PACMPEG.PATHS in the containing directory of 2PACMPEG
- in typical web developer fashion, v3 also brings changes to the user interface (font changed and output window has different colors)
- manual page for yt-dlp <https://man.archlinux.org/man/extra/yt-dlp/yt-dlp.1.en>

# 3. building <a name="building"></a>

## 3.1. tools <a name="building-tools"></a>
- clang++ (with windows headers and libs included if building for windows)

## 3.2. dependencies <a name="building-dependencies"></a>
- ImGui (included)
- GLFW3 (included)
- note: if you are using gayland then you probably need to edit the build script and replace -lX11 with -lwayland-client or whatever its called idk

## 3.3. actually building <a name="building-fr"></a>
- append containing folder of clang++ to PATH shell environment variable if it's not already there
- on windows, run clang_release.bat and clang_release.sh on linux
- (for debug, run compile_imgui.bat/sh **once** and then build with build.bat/sh)

# 4. credits <a name="credits"></a>
- thanks to mastermind graphix designer Karkagami for the icon(s) <3
- GLFW3 - window creation and rendering backend for ImGui
- ImGui - gui toolkit
- (most importantly) FFmpeg and yt-dlp
