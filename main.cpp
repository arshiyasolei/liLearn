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
#include <stdio.h>
#include <map>
#include <string>
#include <vector>

#include "chess.h"
#ifdef EMSCRIPTEN
#include <emscripten.h>
#endif
// Screen dimension constants

int SCREEN_WIDTH = 512;
int SCREEN_HEIGHT = 512;
std::map<int,SDL_Texture*> images;
float SIZE_H = 64;
float SIZE_W = 64;
int firstClick = 0;
int validMove = 0;
int currentMovedPiece = -1;
int oldMovedPiece = -1;
SDL_Window* window = NULL;
SDL_Renderer* gRenderer = NULL;
chessBoard* board = NULL;
movePiece* p = NULL;
SDL_Texture* loadTexture(std::string path, SDL_Renderer* gRenderer);

int numOptimalMovesToStar(){

    


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

const int whitePieceArr[] = {14, 2, 10, 11, 13, 12};
const int blackPieceArr[] = {7, 1, 6, 4, 5, 3};

std::map<int,SDL_Texture*> fillImages(){
    
    
    std::map<int,SDL_Texture*> a;
    for (int i = 0; i < 6; ++i){
        
        a[whitePieceArr[i]] = loadTexture(pieceNamesPng[whitePieceArr[i]],
                                      gRenderer);

        a[blackPieceArr[i]] = loadTexture(pieceNamesPng[blackPieceArr[i]],
                                gRenderer);
                      

    }
    
    a[99] = loadTexture("images/star.png", gRenderer);
    
    return a;

}


void drawBoardPieces(SDL_Window* window, chessBoard* board,
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

void draw(SDL_Window* window, chessBoard* board, SDL_Renderer* gRenderer) {
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

    // Quit SDL subsystems
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
                printf("shit\n");
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
            if (updateBoard(board, p)) {
                printf("game is over!\n");
            }
            validMove = 0;
        }
        
                if (firstClick && currentMovedPiece != -1) {

                    board->board[p->i][p->j]  = 0;
                    draw(window, board, gRenderer);
                    board->board[p->i][p->j]  = currentMovedPiece;
                    printf("reaching?\n");
                    // draw whatever piece that is being moved
                    // draw the pieces!
                    SDL_Rect mySpritePos = {.y = e.button.y - SIZE_H/2,
                                          .x = e.button.x - SIZE_W/2,
                                          .h = SIZE_H,
                                          .w = SIZE_W};

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

int main(int argc, char* args[]) {
    chessBoard boarda = {.castleStats = {0, 0, 0, 0},
                         .lastMovePawnTwoUp = 0,
                         .lastPieceMoveCord = {-1, -1},
                         .turn = 0,
                         .board = {{5, 4, 3, 6, 7, 3, 4, 5},
                                   {1, 1, 1, 1, 1, 1, 1, 1},
                                   {0, 0, 0, 0, 0, 0, 0, 0},
                                   {0, 0, 0, 0, 0, 0, 0, 0},
                                   {0, 0, 0, 99, 0, 0, 0, 0},
                                   {0, 0, 0, 0, 0, 0, 0, 0},
                                   {2, 2, 2, 2, 2, 2, 2, 2},
                                   {10, 11, 12, 13, 14, 12, 11, 10}}};
    // piece to be moved...
    board = &boarda;
    movePiece pa = {1, 0, 3, 0};
    p = &pa;
    int firstClick = 0;
    int validMove = 0;

    // Initialize SDL and SDL_Image
    IMG_Init(IMG_INIT_PNG);
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_TIMER) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
    } else {
        // Create window
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