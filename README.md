# VTFUpscaler

VTFUpscaler is a tool designed for high-quality upscaling of Valve Texture Format (VTF) files. It is primarily meant for mass upscaling in place. This means you can input a directory, and it will upscale and replace all original VTF files in that directory, along with its subdirectories.

## Usage

You can simply pull VTF file or a directory onto VTFUpscaler.exe or open them via the command line.

## Minimum system requirements
Window 10  
DirectX 11

## Compiling

VTFLib is required, but it's not shipped with the project. You can get a precompiled version from https://nemstools.github.io/pages/VTFLib-Download.html. 
You should put VTFLib.h in "$(SolutionDir)\lib". You should place the 64-bit version of VTFLib.lib in "$(SolutionDir)\lib\x64". Finally, VTFLib.dll should be in the same directory as VTFUpscaler.exe. Also, config.txt should be in the same folder as VTFUpscaler.exe.