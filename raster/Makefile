CC = g++
CFLAGS = -std=c++17

INCLUDES =
LDFLAGS = -lSDL2 -lSDL2_image
OS := $(shell uname)
ifeq ($(OS),Darwin)
	INCLUDES += -I/opt/homebrew/include
	LDFLAGS += -L/opt/homebrew/lib
endif

raster: src/raster.cpp
	$(CC) $(CFLAGS) src/raster.cpp $(INCLUDES) $(LDFLAGS) -o bin/raster

display: src/display.cpp
	$(CC) $(CFLAGS) src/display.cpp $(INCLUDES) $(LDFLAGS) -o bin/display

rasterizer: src/rasterizer.cpp
	$(CC) $(CFLAGS) src/rasterizer.cpp $(INCLUDES) $(LDFLAGS) -o bin/rasterizer
