CC=g++
CFLAGS=-std=c++17 -Wall -O2
LDFLAGS=-I/opt/homebrew/include -L/opt/homebrew/lib -lSDL2 -lGLEW -framework OpenGL -framework Cocoa

all: src/main.cpp
	$(CC) $(CFLAGS) src/main.cpp $(LDFLAGS) -o bin/main
