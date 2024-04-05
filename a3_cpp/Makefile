UNAME := $(shell uname)

ifeq ($(UNAME), Linux)
	CC=g++
	CFLAGS=-std=c++17 -Wall -O2
	LDFLAGS=-lGLEW -lGLU -lGL -lSDL2
else
	CC=clang++
	CFLAGS=-std=c++17 -O2 -Wall -I/opt/homebrew/include
	LDFLAGS=-L/opt/homebrew/lib -lSDL2 -lGLEW -framework OpenGL -framework Cocoa
endif

SRC = src/light.cpp src/object.cpp src/camera.cpp src/renderer.cpp src/scene.cpp src/window.cpp src/material.cpp src/debug.cpp
OBJ = $(patsubst src/%.cpp,bin/%.o,$(SRC))
TARGET = bin/e1_1 bin/e1_2 bin/e1_3 bin/e1_4 bin/e1_5 bin/cornell_box bin/cornell_box_transformed
DEBUG = -DDEBUG

all: $(TARGET)

bin/e1_%: $(OBJ) bin/e1_%.o
	$(CC) $(DEBUG) $(CFLAGS) $^ $(LDFLAGS) -o $@

bin/cornell_box: $(OBJ) bin/cornell_box.o
	$(CC) $(DEBUG) $(CFLAGS) $^ $(LDFLAGS) -o $@

bin/cornell_box_transformed: $(OBJ) bin/cornell_box_transformed.o
	$(CC) $(DEBUG) $(CFLAGS) $^ $(LDFLAGS) -o $@

bin/%.o: src/%.cpp
	$(CC) $(DEBUG) $(CFLAGS) -c $< -o $@

clean:
	rm -f bin/*

.PRECIOUS: bin/%.o
