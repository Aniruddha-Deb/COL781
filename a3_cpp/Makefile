UNAME := $(shell uname)

ifeq ($(UNAME), Linux)
	CC=g++
	CFLAGS=-std=c++17 -Wall -O2
	LDFLAGS=-lGLEW -lGLU -lGL -lSDL2
else
	CC=g++
	CFLAGS=-std=c++17 -Wall -O2 -I/opt/homebrew/include
	LDFLAGS=-L/opt/homebrew/lib -lSDL2 -lGLEW -framework OpenGL -framework Cocoa
endif

SRC = src/main.cpp src/object.cpp src/camera.cpp src/renderer.cpp src/scene.cpp src/window.cpp
OBJ = $(patsubst src/%.cpp,bin/%.o,$(SRC))

all: $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) $(LDFLAGS) -o bin/main

bin/%.o: src/%.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f bin/*
