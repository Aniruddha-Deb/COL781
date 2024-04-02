UNAME := $(shell uname)

ifeq ($(UNAME), Linux)
	CC=g++
	CFLAGS=-std=c++17 -Wall -O2
	LDFLAGS=-lGLEW -lGLU -lGL -lSDL2
else
	CC=g++
	CFLAGS=-std=c++17 -O2 -Wall -I/opt/homebrew/include -fno-stack-protector
	LDFLAGS=-L/opt/homebrew/lib -lSDL2 -lGLEW -framework OpenGL -framework Cocoa
endif

SRC = src/light.cpp src/object.cpp src/camera.cpp src/renderer.cpp src/scene.cpp src/window.cpp src/material.cpp
OBJ = $(patsubst src/%.cpp,bin/%.o,$(SRC))
TARGET = e1_1 e1_2 e1_3 e1_4 e1_5

all: $(TARGET)

e1_%: $(OBJ) bin/e1_%.o
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o bin/$@

bin/%.o: src/%.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f bin/*

.PRECIOUS: bin/%.o
