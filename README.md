# encode
C++ cross-platform MP3 encoder utilising Lame library & POSIX threads.

Features:
* Small, uses modern C++ features (lambdas, auto)
* Performs parallel processing of several files, creating thread pool
* Input files support is limited to 16 bit stereo WAV files, sample rate 44100
* Portable - lame is statically compiled inside binary - one binary for any system

Builds & runs on:
* Linux (tested on Gentoo)
* Windows (tested on Windows 10 + mingw-w64)

# Linux build instructions
Install lame along with it's development package, change to project dir:

`cmake . && make`

The binary will be installed to `bin` folder.

# Windows build instructions
Install msys2, launch mingw64 shell, add compiler and lame using 

`mkdir /mingw64 && pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-lame`

change to project dir and run

`CXX="/mingw64/bin/g++" cmake . && make`

The binary will also be installed to `bin` folder.

# Usage
Start program from anywhere, supplying input/output directory with wav files as parameter:

`./encoder /path/to/my/wav/files`

