
all:
	g++ -g -std=c++11 *.cpp -o liLearn -lSDL2 -lSDL2_image -lSDL2_mixer
emc:
	emcc -std=c++11 *.cpp -s USE_SDL=2 -s USE_SDL_IMAGE=2 -s USE_SDL_MIXER=2 -s SDL2_IMAGE_FORMATS='["png"]' \
    --preload-file images --preload-file sounds -o lilearn.html