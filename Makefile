DIRS:=. render util sound random
SRC:=$(wildcard $(addsuffix /*.cpp,$(DIRS)))
OBJ:=$(patsubst %.cpp,obj/%.o,$(SRC))
ODIR:=obj
ODIRS:=$(addprefix $(ODIR)/, $(DIRS))
BIN:=sibbe

SDL_CONFIG=sdl-config
BASEFLAGS:=-Wall -Wextra -std=c++0x -I. -MMD $(shell $(SDL_CONFIG) --cflags)
DFLAGS:=-g
OFLAGS:=-O2
OPTFLAGS=$(DFLAGS)
CXXFLAGS:=$(BASEFLAGS) $(OPTFLAGS)
#CXXFLAGS:=$(BASEFLAGS) $(OFLAGS)
CXX=clang++
GLLIBS:=-lGL -lGLU
LIBS:=$(shell $(SDL_CONFIG) --libs) $(GLLIBS) -lSDL_ttf -lSDL_image

.PHONY: all clean

all: $(BIN)

$(BIN): $(ODIRS) $(OBJ)
	$(CXX) -o "$@" $(OBJ) $(CXXFLAGS) $(LIBS)

$(BIN).html: $(ODIRS) $(OBJ)
	$(CXX) -o "$@" $(OBJ) $(CXXFLAGS) --preload-file data --preload-file score --preload-file sound --preload-file shaders -s TOTAL_MEMORY=520093696

$(OBJ): $(ODIR)/%.o: %.cpp
	$(CXX) "$<" -c -o "$@" $(CXXFLAGS)

clean:
	rm -rf "$(ODIR)"

$(ODIRS):
	mkdir -p "$@"

include $(wildcard $(addsuffix /*.d,$(ODIRS)))
