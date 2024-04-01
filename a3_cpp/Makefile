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

SRC = src/e1_1.cpp src/e1_2.cpp src/e1_3.cpp src/light.cpp src/object.cpp src/camera.cpp src/renderer.cpp src/scene.cpp src/window.cpp
HEADERS = src/object.cpp src/camera.cpp src/renderer.cpp src/scene.cpp src/window.cpp src/light.cpp
OBJ = $(patsubst src/%.cpp,bin/%.o,$(SRC))
HEADEROBJS = $(patsubst src/%.cpp,bin/%.o,$(HEADERS))
TARGET = e1_1 e1_2 e1_3

all: $(TARGET)

e1_1: $(OBJ)
	$(CC) $(CFLAGS) bin/e1_1.o $(HEADEROBJS) $(LDFLAGS) -o bin/e1_1

e1_2: $(OBJ)
	$(CC) $(CFLAGS) bin/e1_2.o $(HEADEROBJS) $(LDFLAGS) -o bin/e1_2

e1_3: $(OBJ)
	$(CC) $(CFLAGS) bin/e1_3.o $(HEADEROBJS) $(LDFLAGS) -o bin/e1_3

bin/%.o: src/%.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f bin/*
