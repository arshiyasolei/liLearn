
all:
	g++ -g -std=c++11 *.cpp -o liLearn -lSDL2 -lSDL2_image
emc:
	emcc *.cpp -s USE_SDL=2 -s USE_SDL_IMAGE=2 -s SDL2_IMAGE_FORMATS='["png"]' \
    --preload-file images -o lilearn.html