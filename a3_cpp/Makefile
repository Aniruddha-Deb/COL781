UNAME := $(shell uname)

ifeq ($(UNAME), Linux)
	CC=g++
	CFLAGS=-std=c++17 -Wall -O2
	LDFLAGS=-lGLEW -lGLU -lGL -lSDL2
else
	CC=g++
	CFLAGS=-std=c++17 -Wall -O2
	LDFLAGS=-I/opt/homebrew/include -L/opt/homebrew/lib -lSDL2 -lGLEW -framework OpenGL -framework Cocoa
endif

all: src/main.cpp
	$(CC) $(CFLAGS) $< $(LDFLAGS) -o bin/main
