/*
css element needed for fullscreen
#canvas {
    position: absolute;
    top: 0px;
    left: 0px;
    margin: 0px;
    width: 100%;
    height: 100%;
    overflow: hidden;
    display: block;
}

js shit

      // Work-around chromium autoplay policy
      // https://github.com/emscripten-core/emscripten/issues/6511
      function resumeAudio(e) {
          if (typeof Module === 'undefined'
              || typeof Module.SDL2 == 'undefined'
              || typeof Module.SDL2.audioContext == 'undefined')
              return;
          if (Module.SDL2.audioContext.state == 'suspended') {
              Module.SDL2.audioContext.resume();
          }
          if (Module.SDL2.audioContext.state == 'running') {
              document.getElementById('canvas').removeEventListener('click',
resumeAudio); document.removeEventListener('keydown', resumeAudio);
          }
      }
      document.getElementById('canvas').addEventListener('click', resumeAudio);
      document.addEventListener('keydown', resumeAudio);

*/

/* TODOS :

Show all legal moves with a dot
Highlight wrong moves with red
Highlight last made move

*/

// Using SDL and standard IO
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <stdio.h>
#include <stdlib.h> /* srand, rand */
#include <time.h>   /* time */

#include <array>
#include <map>
#include <queue>
#include <stack>
#include <string>
#include <unordered_map>
#include <vector>

#include "board.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

// hashing struct for pairs
struct hash_pair {
    size_t operator()(std::pair<int, int> p) const {
        return size_t(p.first) << 32 | p.second;
    }
};

// globals 
int SCREEN_WIDTH = 512;
int SCREEN_HEIGHT = 512;
std::unordered_map<int, SDL_Texture*> images;
int optimal_star = 0;
int num_stars_in_board = 0;
int cur_move_count = 0;
int cur_star_count = 0;
float SIZE_H = 64;
float SIZE_W = 64;
int firstClick = 0;
int validMove = 0;
int currentMovedPiece = -1;
int oldMovedPiece = -1;
int firstTimePlayingFlag = 1;

// Global state for SDL window/render/sound/etc
SDL_Window* window = NULL;
SDL_Renderer* gRenderer = NULL;
liBoard* board = NULL;
movePiece* p = NULL;
Mix_Chunk* moveSound = NULL;
Mix_Chunk* captureSound = NULL;
Mix_Chunk* winSound = NULL;

SDL_Texture* loadTexture(std::string path, SDL_Renderer* gRenderer);
void boardSetup();

// hashes the board to a string
std::string hashBoard(std::array<std::array<int, 8>, 8>& myBoard) {
    int res = 0;
    std::string em = "";
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            em += std::to_string(myBoard[i][j]);
        }
    }
    return em;
}

// returns all the possible board combinations 
std::vector<movePiece> possibleMoves() {
    std::vector<movePiece> moves;
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            for (int k = 0; k < 8; ++k) {
                for (int l = 0; l < 8; ++l) {
                    if (i != k || j != l) {
                        movePiece tempMove = {i, j, k, l};
                        moves.push_back(tempMove);
                    }
                }
            }
        }
    }
    return moves;
}

// calculates the number of moves to optimally collect all stars
int numOptimalMovesToStar() {
    // pair of (num stars collected , board)
    int maxStacksize = 1;

    std::unordered_map<std::string, int> visited;
    std::queue<std::tuple<int, int, std::array<std::array<int, 8>, 8>>> stack;

    stack.push(std::make_tuple(0, 0, board->board));
    int minNum = 2147483647;
    while (!stack.empty()) {
        maxStacksize = std::max(maxStacksize, (int)stack.size());
        // get current board
        liBoard curBoard;
        curBoard.board = std::get<2>(stack.front());
        // if board is not in visited
        std::string resHash = hashBoard(curBoard.board);
        if (!(visited.find(resHash) != visited.end())) {
            // add to visited

            int curStarcount = std::get<0>(stack.front());
            int curMoveCount = std::get<1>(stack.front());
            visited[resHash] = curMoveCount;
            if (curStarcount == num_stars_in_board) {
                minNum = std::min(std::get<1>(stack.front()), minNum);
                printf("max size of stack: %d ,", maxStacksize);
                printf("size of visited %d\n", visited.size());
                return minNum;
            }
            stack.pop();
            for (auto tempMove : possibleMoves()) {
                if ((tempMove.i != tempMove.goalI ||
                     tempMove.j != tempMove.goalJ) &&
                    (curBoard.board[tempMove.i][tempMove.j] &&
                     curBoard.board[tempMove.i][tempMove.j] != 99 &&
                     curBoard.validateMove(&tempMove))) {
                    // add this to stack
                    liBoard backup = {.board = curBoard.board};
                    int starFlag =
                        (curBoard.board[tempMove.goalI][tempMove.goalJ] == 99)
                            ? 1
                            : 0;
                    curBoard.updateBoard(&tempMove);
                    std::queue<std::tuple<int, int, std::array<std::array<int, 8>, 8>>> nq;
                    if (starFlag) {
                        nq.push(std::make_tuple(curStarcount + starFlag,
                            curMoveCount + 1,
                            curBoard.board));
                        stack = nq;
                        curBoard.board = backup.board;
                        break;
                    } 
                    stack.push(std::make_tuple(curStarcount + starFlag,
                                               curMoveCount + 1,
                                               curBoard.board));

                    curBoard.board = backup.board;
                }
            }

        } else {
            // if position of board is
            stack.pop();
        }
    }
    printf("size of visited %d\n", visited.size());
    return minNum;
}

char* pieceNamesPng[] = {"",
                         "images/black_pawn.png",
                         "images/white_pawn.png",
                         "images/black_bishop.png",
                         "images/black_knight.png",
                         "images/black_rook.png",
                         "images/black_queen.png",
                         "images/black_king.png",
                         "images/",
                         "images/",
                         "images/white_rook.png",
                         "images/white_knight.png",
                         "images/white_bishop.png",
                         "images/white_queen.png",
                         "images/white_king.png"};

const int whitePieceArr[] = {10, 11, 13};
const int blackPieceArr[] = {6, 4, 5};

// set up the images (and sound) of the game!
std::unordered_map<int, SDL_Texture*> fillImages() {
    std::unordered_map<int, SDL_Texture*> a;
    for (int i = 0; i < 3; ++i) {
        a[whitePieceArr[i]] =
            loadTexture(pieceNamesPng[whitePieceArr[i]], gRenderer);

        a[blackPieceArr[i]] =
            loadTexture(pieceNamesPng[blackPieceArr[i]], gRenderer);
    }

    a[99] = loadTexture("images/star.png", gRenderer);

    // also load music here
    winSound = Mix_LoadWAV("sounds/win.wav");
    captureSound = Mix_LoadWAV("sounds/capture.wav");
    moveSound = Mix_LoadWAV("sounds/move.wav");

    return a;
}

void drawBoardPieces(SDL_Window* window, liBoard* board,
                     SDL_Renderer* gRenderer) {
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; j++) {
            if (board->board[i][j]) {
                // draw the pieces!
                SDL_Rect mySpritePos = {.x = (int)((j)*SIZE_W + SIZE_W / 128),
                                        .y = (int)((i)*SIZE_H + SIZE_H / 128),
                                        .w = (int)SIZE_W,
                                        .h = (int)SIZE_H};

                SDL_Texture* img;
                if (board->board[i][j] != 99)
                    img = images[board->board[i][j]];
                else
                    img = images[99];
                // draw texture at location
                SDL_RenderCopy(gRenderer, img, NULL, &mySpritePos);
            }
        }
    }
}

void draw(SDL_Window* window, liBoard* board, SDL_Renderer* gRenderer) {
    // prints board
    int alt = 0;
    for (int i = 0; i < 8; ++i) {
        alt = i % 2;
        for (int j = 0; j < 8; ++j) {
            SDL_FRect myRecPos = {
                .x = (j)*SIZE_W, .y = (i)*SIZE_H, .w = SIZE_W, .h = SIZE_H};

            if (alt) {
                if (j % 2 == 0) {
                    SDL_SetRenderDrawColor(gRenderer, 67, 166, 186, 0xFF);
                } else {
                    SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
                }
            } else {
                if (j % 2) {
                    SDL_SetRenderDrawColor(gRenderer, 67, 166, 186, 0xFF);
                } else {
                    SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
                }
            }
            SDL_RenderFillRectF(gRenderer, &myRecPos);
        }
    }
    
    drawBoardPieces(window, board, gRenderer);
}

SDL_Texture* loadTexture(std::string path, SDL_Renderer* gRenderer) {
    // The final texture
    SDL_Texture* newTexture = NULL;

    // Load image at specified path
    SDL_Surface* loadedSurface = IMG_Load(path.c_str());
    if (loadedSurface == NULL) {
        printf("Unable to load image %s! SDL_image Error: %s\n", path.c_str(),
               IMG_GetError());
    } else {
        // Create texture from surface pixels
        newTexture = SDL_CreateTextureFromSurface(gRenderer, loadedSurface);
        if (newTexture == NULL) {
            printf("Unable to create texture from %s! SDL Error: %s\n",
                   path.c_str(), SDL_GetError());
        }

        // Get rid of old loaded surface
        SDL_FreeSurface(loadedSurface);
    }

    return newTexture;
}

void close(SDL_Window* gWindow, SDL_Renderer* gRenderer) {
    // Destroy window
    SDL_DestroyRenderer(gRenderer);
    SDL_DestroyWindow(gWindow);
    gWindow = NULL;
    gRenderer = NULL;
    for (auto const& x : images) {
        SDL_DestroyTexture(x.second);
    }
    Mix_FreeChunk(winSound);
    Mix_FreeChunk(captureSound);
    Mix_FreeChunk(moveSound);
    // Quit SDL subsystems
    Mix_Quit();
    IMG_Quit();
    SDL_Quit();
}

// the main loop of the program that runs every frame
#ifdef __EMSCRIPTEN__
void mainloop() {
#else
int mainloop() {
#endif
    SDL_Event e;
    bool quit = false;

    while (SDL_PollEvent(&e)) {
        int touchFlag = 0;
        if (e.type == SDL_QUIT) {
            close(window, gRenderer);
#ifdef __EMSCRIPTEN__
            emscripten_cancel_main_loop();
#else
            return 0;
#endif
        }

        if (e.type == SDL_WINDOWEVENT) {
            if (e.window.event == SDL_WINDOWEVENT_RESIZED) {
                printf("MESSAGE: Resizing window...\n");
                SIZE_W = e.window.data1 / 8.0;
                SIZE_H = e.window.data2 / 8.0;
                printf("%f %f\n", SIZE_W, SIZE_H);
#ifndef __EMSCRIPTEN__
                return 1;
#endif
            }
        }
        if( e.type == SDL_FINGERDOWN ){
                if (firstClick == 0) {
                    // touch
                    printf("one\n");
                    p->j = (int)(((int)(e.tfinger.x*SIZE_W)) / SIZE_W);
                    p->i = (int)(((int)(e.tfinger.y*SIZE_H)) / SIZE_H);
                    if (board->board[p->i][p->j]) {
                        currentMovedPiece = board->board[p->i][p->j];
                    } else {
                        currentMovedPiece = -1;
                    }
                    printf("%d %d\n", e.tfinger.x, e.tfinger.y);
                    firstClick = 1;
                }
        }
        
        if (e.type == SDL_MOUSEBUTTONDOWN && e.button.which != SDL_TOUCH_MOUSEID) {
            if (e.button.button == SDL_BUTTON_LEFT) {
                if (firstClick == 0) {
                    // see where this click falls on 8x8
                    // this sets the i,j
                    // printf("%d %d\n", (int) (event.mouseButton.x
                    // / 128), (int) (event.mouseButton.y / 128));
                    printf("one\n");
                    p->j = (int)(e.button.x / SIZE_W);
                    p->i = (int)(e.button.y / SIZE_H);
                    if (board->board[p->i][p->j]) {
                        currentMovedPiece = board->board[p->i][p->j];
                    } else {
                        currentMovedPiece = -1;
                    }
                    printf("%d %d\n", e.button.x, e.button.y);
                    firstClick = 1;
                }
                // printf("%d %d\n", event.mouseButton.x,
                // event.mouseButton.y);
            }
        }
        if (e.type == SDL_FINGERUP) {
                if (firstClick == 1) {
                    printf("two\n");
                    if (p->i != (int)(((int)(e.tfinger.y*SIZE_H)) / SIZE_H) ||
                        (int)(((int)(e.tfinger.x*SIZE_W)) / SIZE_W) != p->j) {

                        p->j = (int)(((int)(e.tfinger.x*SIZE_W)) / SIZE_W);
                        p->i = (int)(((int)(e.tfinger.y*SIZE_H)) / SIZE_H);
                        validMove = 1;
                    }
                    firstClick = 0;
                    // this sets the goal i,j
                }
        }
        if (e.type == SDL_MOUSEBUTTONUP && e.button.which != SDL_TOUCH_MOUSEID) {
            if (e.button.button == SDL_BUTTON_LEFT) {
                if (firstClick == 1) {
                    printf("two\n");
                    if (p->i != (int)(e.button.y / SIZE_H) ||
                        (int)(e.button.x / SIZE_W) != p->j) {
                        p->goalJ = (int)(e.button.x / SIZE_W);
                        p->goalI = (int)(e.button.y / SIZE_H);
                        validMove = 1;
                    }
                    firstClick = 0;
                    // this sets the goal i,j
                }
                // printf("%d %d\n", event.mouseButton.x,
                // event.mouseButton.y);
            }
        }

        if (e.type == SDL_MOUSEMOTION) {
            if (e.button.button == SDL_BUTTON_LEFT) {
            }
        }

        if( e.type == SDL_FINGERMOTION ){ 
            touchFlag = 1;
        }
        // Clear screen
        SDL_RenderClear(gRenderer);

        // Render texture to screen
        // SDL_RenderCopy( gRenderer, testTexture, NULL, NULL );
        if (validMove) {
            if (board->validateMove(p)) {
                if (board->board[p->goalI][p->goalJ] == 99) {
                    cur_star_count += 1;
                    Mix_PlayChannel(-1, captureSound, 0);
                } else {
                    Mix_PlayChannel(-1, moveSound, 0);
                }
                cur_move_count += 1;
                board->updateBoard(p);
                if (cur_star_count == num_stars_in_board) {
                    char buffer[100];
                    Mix_PlayChannel(-1, winSound, 0);

                    snprintf(buffer, 100,
                             "Congrats! it took you %d moves. Optimal was %d "
                             "moves!\n",
                             cur_move_count, optimal_star);
                    cur_move_count = 0;
                    cur_star_count = 0;
                    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION,
                                             "Update", buffer, NULL);
                    boardSetup();
                    optimal_star = numOptimalMovesToStar();
                    char buffer2[100];
                    snprintf(buffer2, 100, "Number of optimal Moves: %d \n",
                             optimal_star);
                    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION,
                                             "Update", buffer2, NULL);
                }
            }
            validMove = 0;
        }

        if (firstClick && currentMovedPiece != -1) {
            board->board[p->i][p->j] = 0;
            draw(window, board, gRenderer);
            board->board[p->i][p->j] = currentMovedPiece;

            // draw whatever piece that is being moved
            // draw the pieces!
            int x = 0;
            int y = 0;
            // if we had touch..
            if (!touchFlag) {
                x = e.button.x - (int)(SIZE_W / 2);
                y = e.button.y - (int)(SIZE_H / 2);
            } else {
                x = (int)(e.tfinger.x*SIZE_W) - (int)(SIZE_W / 2);
                y = (int)(e.tfinger.y*SIZE_H) - (int)(SIZE_H / 2);
            }
            SDL_Rect mySpritePos = {.x = x,
                                    .y = y,
                                    .w = (int)SIZE_W,
                                    .h = (int)SIZE_H};
            
            SDL_Texture* img;
            if (currentMovedPiece != 99)
                img = images[currentMovedPiece];
            else
                img = images[99];
            // draw texture at location
            SDL_RenderCopy(gRenderer, img, NULL, &mySpritePos);

        } else {
            draw(window, board, gRenderer);
        }
        // Update screen
        SDL_RenderPresent(gRenderer);
    }
#ifndef __EMSCRIPTEN__
    return 1;
#endif

    // SDL_GetRendererOutputSize(gRenderer, &ab, &ac);
    // printf("%d %d\n",ab,ac);
}

// function that sets up board randomly
void boardSetup() {
    num_stars_in_board = (rand() % 30) + 12;
    std::vector<std::pair<int, int>> starPairs;
    std::unordered_map<std::pair<int, int>, int, hash_pair> visi;
    int random_piece_choice = rand() % 3;
    random_piece_choice = 1;
    int mainPieceI = rand() % 8;
    int mainPieceJ = rand() % 8;
    visi[std::make_pair(mainPieceI, mainPieceJ)] = 0;
    for (int v = 0; v < num_stars_in_board; ++v) {
        std::pair<int, int> sample(rand() % 8, rand() % 8);
        while ((visi.find(sample) != visi.end())) {
            std::pair<int, int> t(rand() % 8, rand() % 8);
            sample = t;
        }
        visi[sample] = 0;
        starPairs.push_back(sample);
    }
    if (firstTimePlayingFlag) {
        firstTimePlayingFlag = 0;
    }

    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            if (i == mainPieceI && j == mainPieceJ) {
                board->board[i][j] = whitePieceArr[random_piece_choice];
            } else {
                board->board[i][j] = 0;
            }
        }
    }
    for (int i = 0; i < num_stars_in_board; ++i) {
        board->board[starPairs[i].first][starPairs[i].second] = 99;
    }
}
int main(int argc, char* args[]) {
    srand(time(NULL));
    liBoard boarda;
    // piece to be moved...
    board = &boarda;
    boardSetup();

    movePiece pa = {1, 0, 3, 0};
    p = &pa;
    int firstClick = 0;
    int validMove = 0;
    optimal_star = numOptimalMovesToStar();
    char buffer[100];
    snprintf(buffer, 100, "Number of optimal Moves: %d \n", optimal_star);
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Update", buffer,
                             NULL);

    printf("num optimal Moves %d \n", optimal_star);
    // Initialize SDL and SDL_Image
    IMG_Init(IMG_INIT_PNG);
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_TIMER |
                 SDL_INIT_AUDIO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
    } else {
        // Create window
        if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
            printf("SDL_mixer could not initialize! SDL_mixer Error: %s\n",
                   Mix_GetError());
        }
        window = SDL_CreateWindow("LiLearn", SDL_WINDOWPOS_UNDEFINED,
                                  SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH,
                                  SCREEN_HEIGHT,
                                  SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL| SDL_WINDOW_RESIZABLE);
        
        if (window == NULL) {
            printf("Window could not be created! SDL_Error: %s\n",
                   SDL_GetError());
        } else {
            
            SDL_SetWindowSize(window, SCREEN_WIDTH, SCREEN_HEIGHT);
            gRenderer =
                SDL_CreateRenderer(window, -1,SDL_RENDERER_ACCELERATED);
            SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
            images = fillImages();

            // testTexture = loadTexture("images/black_pawn.png",gRenderer);
            // handle events
        }
    }
#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(mainloop, 0, 1);
#else
    while (mainloop()) {
    }
#endif

    return 0;
}