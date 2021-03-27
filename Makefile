
all:
	#g++-10 -fsanitize=leak -g -std=c++11 main.cpp -o liLearn 

	g++ -g -O3 -std=c++11 main.cpp -o liLearn -lSDL2 -lSDL2_image -lSDL2_mixer 
emc:
	emcc -O3 -s ASSERTIONS=1 -s ALLOW_MEMORY_GROWTH=1  -std=c++11 main.cpp -s USE_SDL=2 -s USE_SDL_IMAGE=2 -s USE_SDL_MIXER=2 -s SDL2_IMAGE_FORMATS='["png"]' \
    --preload-file images --preload-file sounds -o index.html