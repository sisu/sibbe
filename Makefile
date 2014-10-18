DIRS:=. render util sound random
SRC:=$(wildcard $(addsuffix /*.cpp,$(DIRS)))
OBJ:=$(patsubst %.cpp,obj/%.o,$(SRC))
ODIR:=obj
ODIRS:=$(addprefix $(ODIR)/, $(DIRS))
BIN:=sibbe

BASEFLAGS:=-Wall -Wextra -std=c++0x -I. -MMD
DFLAGS:=-g
OFLAGS:=-O2
CXXFLAGS:=$(BASEFLAGS) $(DFLAGS)
#CXXFLAGS:=$(BASEFLAGS) $(OFLAGS)
CXX=clang++
GLLIBS:=-lGL -lGLU
LIBS:=$(shell sdl-config --libs) $(GLLIBS) -lSDL_ttf -lSDL_image

.PHONY: all clean

all: $(BIN)

$(BIN): $(ODIRS) $(OBJ)
	$(CXX) -o "$@" $(OBJ) $(CXXFLAGS) $(LIBS)

$(BIN).html: $(ODIRS) $(OBJ)
	$(CXX) -o "$@" $(OBJ) $(CXXFLAGS)

$(OBJ): $(ODIR)/%.o: %.cpp
	$(CXX) "$<" -c -o "$@" $(CXXFLAGS)

clean:
	rm -rf "$(ODIR)"

$(ODIRS):
	mkdir -p "$@"

include $(wildcard $(addsuffix /*.d,$(ODIRS)))
