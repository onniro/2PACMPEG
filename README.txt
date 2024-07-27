2PACMPEG - cool front-end for FFmpeg

description:
    the purpose of this program is to make it easier and faster for
    me and some friends to send shadowplay clips and whatnot to each other
    on discord. usage of this software for non-zoomer purposes is against
    terms of service

installation for with FFmpeg release:
    step 1: download the archive where it says "2PACMPEG with FFmpeg" from 
            https://github.com/onniro/2PACMPEG/releases
    step 2: unpack the archive 

installation for (no FFmpeg) release:
    step 1: obtain copies of the FFmpeg binaries (namely ffmpeg and ffprobe)
    step 2: follow the "with FFmpeg release" installation instructions, 
            but instead of downloading 2PACMPEG with FFmpeg, download
            2PACMPEG (no FFmpeg)
    step 3: go to the containing folder of the 2PACMPEG executable
    step 4: make a folder there with the name ffmpeg
    step 5: copy (or move) the ffmpeg and ffprobe executables into that folder 

usage:
    -   go to the folder where you unpacked the archive and click on where 
        it says 2PACMPEG.EXE
    -   click on "select input file" to select input file (or type/paste in the path 2 tha file)
    -   enter arguments for FFmpeg (you can do this by clicking on one 
        of the presets on the right panel)
    -   to make a new preset, enter arguments into the "args for FFmpeg" -field
        and click "add to argument presets" to add them to the argument presets
    -   enter the full path to the file that you want FFmpeg to output 
        (eg. c:\stuff\video.mp4)
    -   you can also set a default folder that you can easily enter in 
        with the click of a button by typing in the default output path 
        field and then clicking on where it says "set as default folder"
    note: if you have not set a default path and also only specify 
        the output filename, the file will be output to the containing 
        folder of the ffmpeg executable
    -   click on start to launch FFmpeg 

credits:
    Karkagami - made the icon (/misc/FFMPAC.ico)
