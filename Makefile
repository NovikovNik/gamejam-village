CC = g++
LANG_STD = -std=c++17
CFLAGS = -Wall -Wfatal-errors
INCLUDE_PATH = -I"./libs/"
SRC_FILES = ./src/Main.cpp ./src/*/*.cpp ./libs/imgui/*.cpp
LINKER_FLAGS = -lglfw -llua -lSDL2_Image -lSDL2_ttf -lSDL2_Mixer
SDL_FLAGS = `sdl2-config --libs --cflags`
OBJ_NAME = game_engine

build:
	$(CC) $(LANG_STD) $(CFLAGS) $(INCLUDE_PATH) $(SRC_FILES) $(SDL_FLAGS) $(LINKER_FLAGS) -o $(OBJ_NAME)

run:
	./$(OBJ_NAME)

clean:
	rm $(OBJ_NAME)
