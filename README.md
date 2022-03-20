# ysfx

Hosting library and audio plugin for JSFX

![capture](docs/capture.png)

## Description

This package provides support for audio and MIDI effects developed with the JSFX
language. These effects exist in source code form, and they are compiled and ran
natively by hosting software.

This contains a hosting library, providing a JSFX compiler and runtime.
In addition, there is an audio plugin which can act as a JSFX host in a digital
audio workstation.

## Remarks

Despite what the name might suggest, JSFX is not JavaScript.
These technologies are unrelated to each other.

This project is not the work of Cockos, Inc; however, it is based on several
free and open source components from the WDL. Originally, this project was based
on jsusfx by Pascal Gauthier, but then it became an entire rewrite made from
scratch.

Some time after the realization of this project, Cockos announced the release of
JSFX as open source under the LGPL. This does not affect the development of this
project, which remains a custom implementation based on the liberally licensed
bits of the WDL.

# Usage notes

The audio plugin will initially present the JSFX effects available in the REAPER
user folders, if these exist.

The effects are source code files which end with the extension `.jsfx`, or with
no extension at all.

Note that, unlike REAPER, ysfx will let you install JSFX wherever you want.
If you use effects from a custom folder, ysfx will usually figure things out, but
not always.

The ideal hierarchy is one where there exists at least a pair of folders named
"Effects" and "Data" side-by-side, which receive the code files and resource files
respectively.

Example:
- `My JSFX/Effects/guitar/amp-model`
- `My JSFX/Data/amp_models/Marshall JCM800 - Marshall Stock 70.wav`

## Download development builds

- [32-bit Windows VST3](https://nightly.link/jpcima/ysfx/workflows/build/master/Windows%2032-bit%20VST3.zip)
- [64-bit Windows VST3](https://nightly.link/jpcima/ysfx/workflows/build/master/Windows%2064-bit%20VST3.zip)
- [64-bit GNU/Linux VST3](https://nightly.link/jpcima/ysfx/workflows/build/master/Linux%2064-bit%20VST3.zip)
- [macOS Universal AU (**self-signed**)](https://nightly.link/jpcima/ysfx/workflows/build/master/macOS%20AU.zip)
- [macOS Universal VST3 (**self-signed**)](https://nightly.link/jpcima/ysfx/workflows/build/master/macOS%20VST3.zip)

## Building

To build the project, one must first set up a C++ development environment
equipped with Git and CMake. One can then build the library and the audio
plugin, by entering commands as follows:

```
git clone https://github.com/jpcima/ysfx.git
cd ysfx
git submodule update --init --recursive
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo ..
cmake --build .
```

## JSFX resources

Great free and open-source effects:

- Saike: https://github.com/JoepVanlier/JSFX
- Geraint Luff: https://github.com/geraintluff/jsfx
- Justin Johnson: https://github.com/Justin-Johnson/ReJJ
