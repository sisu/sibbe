#!/bin/bash
make clean
make CXX=i686-w64-mingw32-g++ SDL_CONFIG=i686-w64-mingw32-sdl-config GLLIBS="-lglew32 -lopengl32 -lglu32" OPTFLAGS="-O2 -DUSE_GLEW" BIN=sibbe.exe $@
