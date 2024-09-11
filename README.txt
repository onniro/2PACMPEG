2PACMPEG - cool front-end for FFmpeg

0. yapping
    this program was written with the intention of using it to send
    shadowplay clips and whatnot to friends on discord without having
    to waste money on nitro.

1.1 installation for 2PACMPEG_with_FFmpeg
    step 1: download the archive where it says "2PACMPEG_with_FFmpeg" from 
            https://github.com/onniro/2PACMPEG/releases
    step 2: unpack the archive 

1.2 installation for 2PACMPEG_no_FFmpeg
    step 1: obtain copies of the FFmpeg binaries (namely ffmpeg and ffprobe)
    step 2: follow the instructions in "installation for 2PACMPEG_with_FFmpeg", 
            but instead of downloading 2PACMPEG_with_FFmpeg, download 
            2PACMPEG_no_FFmpeg instead
    step 3: go to the containing folder of the 2PACMPEG executable
    step 4: make a folder there with the name ffmpeg
    step 5: copy (or move) the ffmpeg and ffprobe executables into that folder 

2. usage:
    -   go to the folder where you unpacked the archive and click on where 
        it says 2PACMPEG.EXE
    -   click on "select input file" to select input file (or type/paste in the path 2 tha file)
    -   enter arguments for FFmpeg (you can do this by clicking on one 
        of the presets on the right panel)
    -   to make a new preset, enter arguments into the "args for FFmpeg" -field
        and click "add to argument presets" to add them to the argument presets
    -   enter the full path to the file that you want FFmpeg to output 
        (eg. c:\stuff\video.mp4)
    -   you can also set a default output folder, so that you only have to 
        specify the name of the file, instead of the full path. to do this
        you have to type in the path to the folder under the text field labeled
        "default output folder" and then click "set as default folder"
    note: if you have not set a default path and also only specify 
        the output filename, the file will be output to the containing 
        folder of the ffmpeg executable
    -   click on start to launch FFmpeg 

3. build dependencies 
    - clang++ for x86_64 with windows headers and libs (i use ucrt but probably doesnt matter tbh)
      (e.g https://github.com/mstorsjo/llvm-mingw)
    - imgui (included)
    - glfw3 (included)

4. credits 
    thanks to mastermind graphix designer Karkagami for the icon(s) <3
    glfw3 - window creation and rendering backend for imgui
    imgui - gui toolkit

