# Computer Health System

Software for planing breaks at computer work.

# System compatibility

Software is programmed in c99 with GTK+-3.0 toolkit, system is compatible with GNU Linux and MS Windows 7 and newer.

# Instalation

On Linux system is needed to install gtk development library and gcc compiler. To build from source code simply run build script:

$ sh build -p lin64/lin32

On MS Windows is the simplest way to build from souce code install MSYS software package and install GTK development libraries and for running the GTK application install GTK+-3.0 runtime libraries. After install development libraries run build script:

$ sh build -p win64/win32
