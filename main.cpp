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

*/

/* TODOS :

Add sound effects
Git gud
Show all legal moves with a dot
Highlight wrong moves with red
Highlight last made move
Add Chewt's UCI

*/

// Using SDL and standard IO
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <stdio.h>
#include <stdlib.h> /* srand, rand */
#include <time.h>   /* time */

#include <map>
#include <queue>
#include <stack>
#include <string>
#include <vector>

#include "board.h"

#ifdef EMSCRIPTEN
#include <emscripten.h>
#endif
// Screen dimension constants

int SCREEN_WIDTH = 512;
int SCREEN_HEIGHT = 512;
std::map<int, SDL_Texture*> images;
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
SDL_Window* window = NULL;
SDL_Renderer* gRenderer = NULL;
liBoard* board = NULL;
movePiece* p = NULL;
Mix_Chunk *moveSound = NULL;
Mix_Chunk *captureSound = NULL;
Mix_Chunk *winSound = NULL;
SDL_Texture* loadTexture(std::string path, SDL_Renderer* gRenderer);
void boardSetup();
int numOptimalMovesToStar() {
    // pair of (num stars collected , board)

    std::map<std::vector<std::vector<int>>, int> visited;

    std::queue<std::tuple<int, int, liBoard>> stack;
    stack.push(std::make_tuple(0, 0, *board));
    int minNum = 2147483647;
    while (!stack.empty()) {
        // get current board
        liBoard curBoard;
        curBoard.board = std::get<2>(stack.front()).board;
        // if board is not in visited
        if (!(visited.find(curBoard.board) != visited.end())) {
            // add to visited

            int curStarcount = std::get<0>(stack.front());
            int curMoveCount = std::get<1>(stack.front());
            visited[curBoard.board] = curMoveCount;
            if (curStarcount == num_stars_in_board) {
                minNum = std::min(std::get<1>(stack.front()), minNum);
                return minNum;
            }
            stack.pop();
            for (int i = 0; i < 8; ++i) {
                for (int j = 0; j < 8; ++j) {
                    for (int k = 0; k < 8; ++k) {
                        for (int l = 0; l < 8; ++l) {
                            if (i != k || j != l) {
                                if (curBoard.board[i][j] &&
                                    curBoard.board[i][j] != 99) {
                                    movePiece tempMove = {i, j, k, l};
                                    if (curBoard.validateMove(&tempMove)) {
                                        // add this to stack
                                        liBoard backup;
                                        int starFlag = 0;
                                        backup.board = curBoard.board;
                                        if (curBoard.board[k][l] == 99) {
                                            starFlag = 1;
                                        }
                                        curBoard.updateBoard(&tempMove);
                                        if (starFlag) {
                                            stack.push(std::make_tuple(
                                                curStarcount + 1,
                                                curMoveCount + 1, curBoard));
                                        } else {
                                            stack.push(std::make_tuple(
                                                curStarcount, curMoveCount + 1,
                                                curBoard));
                                        }

                                        curBoard.board = backup.board;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        } else {
            // if position of board is
            visited[curBoard.board] = 0;
            stack.pop();
        }
    }

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

std::map<int, SDL_Texture*> fillImages() {
    std::map<int, SDL_Texture*> a;
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
                .y = (i)*SIZE_H, .x = (j)*SIZE_W, .h = SIZE_H, .w = SIZE_W};

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
#ifdef EMSCRIPTEN
void mainloop() {
#else
int mainloop() {
#endif
    SDL_Event e;
    bool quit = false;

    if (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            close(window, gRenderer);
#ifdef EMSCRIPTEN
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
#ifndef EMSCRIPTEN
                return 1;
#endif
            }
        }
        if (e.type == SDL_MOUSEBUTTONDOWN) {
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

        if (e.type == SDL_MOUSEBUTTONUP) {
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
        // Clear screen
        SDL_RenderClear(gRenderer);

        // Render texture to screen
        // SDL_RenderCopy( gRenderer, testTexture, NULL, NULL );
        if (validMove) {
            if (board->validateMove(p)) {
                if (board->board[p->goalI][p->goalJ] == 99) {
                    cur_star_count += 1;
                    Mix_PlayChannel( -1, captureSound, 0 );
                } else {
                    Mix_PlayChannel( -1, moveSound, 0 );
                }
                cur_move_count += 1;
                board->updateBoard(p);
                if (cur_star_count == num_stars_in_board) {
                    char buffer[100];
                    Mix_PlayChannel( -1, winSound, 0 );
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
                    memset(buffer, 0, sizeof(buffer));
                    snprintf(buffer, 100, "Number of optimal Moves: %d \n",
                             optimal_star);
                    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION,
                                             "Update", buffer, NULL);
                }
            }
            validMove = 0;
        }

        if (firstClick && currentMovedPiece != -1) {
            board->board[p->i][p->j] = 0;
            draw(window, board, gRenderer);
            board->board[p->i][p->j] = currentMovedPiece;
            printf("reaching?\n");
            // draw whatever piece that is being moved
            // draw the pieces!
            SDL_Rect mySpritePos = {.y = e.button.y - (int)(SIZE_H / 2),
                                    .x = e.button.x - (int)(SIZE_W / 2),
                                    .h = (int)SIZE_H,
                                    .w = (int)SIZE_W};

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
#ifndef EMSCRIPTEN
    return 1;
#endif

    // SDL_GetRendererOutputSize(gRenderer, &ab, &ac);
    // printf("%d %d\n",ab,ac);
}

// function that sets up board randomly
void boardSetup() {
    num_stars_in_board = rand() % 2 + 4;
    std::vector<std::pair<int, int>> starPairs;
    std::map<std::pair<int, int>, int> visi;

    int random_piece_choice = rand() % 3;
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
        for (int i = 0; i < 8; ++i) {
            std::vector<int> tempBank;
            for (int j = 0; j < 8; ++j) {
                if (i == mainPieceI && j == mainPieceJ) {
                    tempBank.push_back(whitePieceArr[random_piece_choice]);
                } else {
                    tempBank.push_back(0);
                }
            }
            board->board.push_back(tempBank);
        }
        for (int i = 0; i < num_stars_in_board; ++i) {
            board->board[starPairs[i].first][starPairs[i].second] = 99;
            firstTimePlayingFlag = 0;
        }
    } else {
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
                                  SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

        if (window == NULL) {
            printf("Window could not be created! SDL_Error: %s\n",
                   SDL_GetError());
        } else {
            int ab;
            int ac;
            SDL_SetWindowSize(window, SCREEN_WIDTH, SCREEN_HEIGHT);
            gRenderer =
                SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
            SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
            images = fillImages();

            // testTexture = loadTexture("images/black_pawn.png",gRenderer);
            // handle events
        }
    }
#ifdef EMSCRIPTEN
    emscripten_set_main_loop(mainloop, 0, 1);
#else
    while (mainloop()) {
    }
#endif

    return 0;
}