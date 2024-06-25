#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

// Constants
const int SCREEN_WIDTH = 1000;
const int SCREEN_HEIGHT = 1000;
const int DEFAULT_CREATURE_SIZE = 50;
int CREATURE_SIZE = DEFAULT_CREATURE_SIZE;
const int COIN_SIZE = 50;
const int BLOCKER_SIZE = 20;
const int SPEED = 700;
const int NUM_OF_BLOCKERS = 20;
const int FPS = 120;
const Uint32 FRAME_DELAY = 1000 / FPS;
const int PATTERN_SIZE = 50;
const char *CYCLOPS_IMAGE = "assets/cyclops.png";
const char *COIN_IMAGE = "assets/coin.png";

// Function prototypes
bool initializeSDL(SDL_Window **window, SDL_Renderer **renderer);
void closeSDL(SDL_Window *window, SDL_Renderer *renderer, TTF_Font *font, SDL_Texture *mainTexture, SDL_Texture *coinTexture, SDL_Texture *backgroundTexture);
bool loadMedia(TTF_Font **font, SDL_Texture **mainTexture, SDL_Texture **coinTexture, SDL_Renderer *renderer, const char *fontPath);
SDL_Texture *generateBackgroundTexture(SDL_Renderer *renderer);
void renderText(SDL_Renderer *renderer, TTF_Font *font, const char *text, int x, int y);
SDL_Texture *loadTexture(SDL_Renderer *renderer, const char *path);
void resetGame(int *score, bool *gameOver, float *posX, float *posY, SDL_Rect *coinRect, SDL_Rect *blockers);
bool checkBlockerCollisions(SDL_Rect *rect, SDL_Rect *blockers);
void updatePlayerSize(SDL_Rect *playerRect, float *posX, float *posY, int newSize);
void generateBlockers(SDL_Rect *blockers, int numBlockers, SDL_Rect *playerRect, SDL_Rect *coinRect);

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("Usage: %s <font_path>\n", argv[0]);
        return 1;
    }

    const char *fontPath = argv[1];

    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;
    TTF_Font *font = NULL;
    SDL_Texture *mainTexture = NULL;
    SDL_Texture *coinTexture = NULL;
    SDL_Texture *backgroundTexture = NULL;

    if (!initializeSDL(&window, &renderer))
    {
        return 1;
    }

    if (!loadMedia(&font, &mainTexture, &coinTexture, renderer, fontPath))
    {
        closeSDL(window, renderer, font, mainTexture, coinTexture, backgroundTexture);
        return 1;
    }

    backgroundTexture = generateBackgroundTexture(renderer);
    if (backgroundTexture == NULL)
    {
        printf("Failed to generate background texture!\n");
        closeSDL(window, renderer, font, mainTexture, coinTexture, backgroundTexture);
        return 1;
    }

    SDL_Event e;
    bool quit = false;
    int score = 0;
    bool gameOver = false;

    SDL_Rect playerRect = {SCREEN_WIDTH / 2 - CREATURE_SIZE / 2, SCREEN_HEIGHT / 2 - CREATURE_SIZE / 2, CREATURE_SIZE, CREATURE_SIZE};
    float posX = playerRect.x;
    float posY = playerRect.y;

    srand(time(NULL));
    SDL_Rect coinRect = {rand() % (SCREEN_WIDTH - COIN_SIZE), rand() % (SCREEN_HEIGHT - COIN_SIZE), COIN_SIZE, COIN_SIZE};
    SDL_Rect blockers[NUM_OF_BLOCKERS];

    generateBlockers(blockers, NUM_OF_BLOCKERS, &playerRect, &coinRect);

    Uint32 lastTick = SDL_GetTicks();

    SDL_Rect playAgainButton = {
        SCREEN_WIDTH / 2 - 200,
        SCREEN_HEIGHT / 2 - 25,
        400,
        50};

    while (!quit)
    {
        Uint32 currentTick = SDL_GetTicks();
        float deltaTime = (currentTick - lastTick) / 1000.0f;
        lastTick = currentTick;

        while (SDL_PollEvent(&e) != 0)
        {
            if (e.type == SDL_QUIT)
            {
                quit = true;
            }
            else if (e.type == SDL_MOUSEBUTTONDOWN && gameOver)
            {
                int mouseX, mouseY;
                SDL_GetMouseState(&mouseX, &mouseY);
                if (e.button.button == SDL_BUTTON_LEFT && SDL_PointInRect(&(SDL_Point){mouseX, mouseY}, &playAgainButton))
                {
                    // Reset the game state
                    resetGame(&score, &gameOver, &posX, &posY, &coinRect, blockers);
                    updatePlayerSize(&playerRect, &posX, &posY, DEFAULT_CREATURE_SIZE);
                    playerRect.x = (int)posX;
                    playerRect.y = (int)posY;
                }
            }
        }

        if (!gameOver)
        {
            const Uint8 *currentKeyStates = SDL_GetKeyboardState(NULL);
            float newPosX = posX, newPosY = posY;

            if (currentKeyStates[SDL_SCANCODE_UP])
            {
                newPosY -= SPEED * deltaTime;
            }
            if (currentKeyStates[SDL_SCANCODE_DOWN])
            {
                newPosY += SPEED * deltaTime;
            }
            if (currentKeyStates[SDL_SCANCODE_LEFT])
            {
                newPosX -= SPEED * deltaTime;
            }
            if (currentKeyStates[SDL_SCANCODE_RIGHT])
            {
                newPosX += SPEED * deltaTime;
            }

            // Ensure the player stays within screen boundaries
            if (newPosX < 0)
            {
                newPosX = 0;
            }
            else if (newPosX + CREATURE_SIZE > SCREEN_WIDTH)
            {
                newPosX = SCREEN_WIDTH - CREATURE_SIZE;
            }

            if (newPosY < 0)
            {
                newPosY = 0;
            }
            else if (newPosY + CREATURE_SIZE > SCREEN_HEIGHT)
            {
                newPosY = SCREEN_HEIGHT - CREATURE_SIZE;
            }

            bool collision = false;
            SDL_Rect newPlayerRect = {(int)newPosX, (int)newPosY, CREATURE_SIZE, CREATURE_SIZE};

            for (int i = 0; i < NUM_OF_BLOCKERS; i++)
            {
                if (SDL_HasIntersection(&newPlayerRect, &blockers[i]))
                {
                    collision = true;
                    break;
                }
            }

            if (collision)
            {
                gameOver = true;
            }
            else
            {
                posX = newPosX;
                posY = newPosY;
            }

            playerRect.x = (int)posX;
            playerRect.y = (int)posY;

            if (SDL_HasIntersection(&playerRect, &coinRect))
            {
                score++;
                int newCreatureSize = CREATURE_SIZE + 1; // Increase the player size
                CREATURE_SIZE = newCreatureSize;
                updatePlayerSize(&playerRect, &posX, &posY, newCreatureSize);
                do
                {
                    coinRect.x = rand() % (SCREEN_WIDTH - COIN_SIZE);
                    coinRect.y = rand() % (SCREEN_HEIGHT - COIN_SIZE);
                    coinRect.w = COIN_SIZE;
                    coinRect.h = COIN_SIZE;
                } while (SDL_HasIntersection(&coinRect, &playerRect) || checkBlockerCollisions(&coinRect, blockers));
            }
        }

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 1);
        SDL_RenderClear(renderer);

        // Render background
        SDL_RenderCopy(renderer, backgroundTexture, NULL, NULL);

        // Render main object
        SDL_RenderCopy(renderer, mainTexture, NULL, &playerRect);

        // Render coin
        SDL_RenderCopy(renderer, coinTexture, NULL, &coinRect);

        // Render blockers
        SDL_SetRenderDrawColor(renderer, 64, 64, 64, 1);
        for (int i = 0; i < NUM_OF_BLOCKERS; i++)
        {
            SDL_RenderFillRect(renderer, &blockers[i]);
        }

        // Render score
        renderText(renderer, font, "Score:", 10, 10);
        char scoreText[16];
        sprintf(scoreText, "%d", score);
        renderText(renderer, font, scoreText, 120, 10);

        if (gameOver)
        {
            SDL_SetRenderDrawColor(renderer, 255, 215, 51, 0.8);
            SDL_RenderFillRect(renderer, &playAgainButton);

            int textWidth, textHeight;
            TTF_SizeText(font, "Play Again", &textWidth, &textHeight);
            int textX = playAgainButton.x + (playAgainButton.w - textWidth) / 2;
            int textY = playAgainButton.y + (playAgainButton.h - textHeight) / 2;

            renderText(renderer, font, "Play Again", textX, textY);
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(FRAME_DELAY);
    }

    closeSDL(window, renderer, font, mainTexture, coinTexture, backgroundTexture);
    return 0;
}

SDL_Texture *generateBackgroundTexture(SDL_Renderer *renderer)
{
    SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, SCREEN_WIDTH, SCREEN_HEIGHT);
    if (texture == NULL)
    {
        printf("Failed to create background texture! SDL_Error: %s\n", SDL_GetError());
        return NULL;
    }

    SDL_SetRenderTarget(renderer, texture);
    SDL_SetRenderDrawColor(renderer, 240, 240, 240, 1); // Light grey color in RGBa format
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 230, 230, 230, 1); // Darker grey for the pattern in RGBa format
    for (int y = 0; y < SCREEN_HEIGHT; y += PATTERN_SIZE)
    {
        for (int x = 0; x < SCREEN_WIDTH; x += PATTERN_SIZE)
        {
            if ((x / PATTERN_SIZE) % 2 == (y / PATTERN_SIZE) % 2)
            {
                SDL_Rect rect = {x, y, PATTERN_SIZE, PATTERN_SIZE};
                SDL_RenderFillRect(renderer, &rect);
            }
        }
    }

    SDL_SetRenderTarget(renderer, NULL);
    return texture;
}

void renderText(SDL_Renderer *renderer, TTF_Font *font, const char *text, int x, int y)
{
    SDL_Color textColor = {0, 0, 0, 255};
    SDL_Surface *textSurface = TTF_RenderText_Blended(font, text, textColor);
    if (textSurface == NULL)
    {
        printf("Unable to render text surface! TTF_Error: %s\n", TTF_GetError());
        return;
    }
    SDL_Texture *textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    if (textTexture == NULL)
    {
        printf("Unable to create texture from rendered text! SDL_Error: %s\n", SDL_GetError());
        SDL_FreeSurface(textSurface);
        return;
    }
    SDL_Rect renderQuad = {x, y, textSurface->w, textSurface->h};
    SDL_RenderCopy(renderer, textTexture, NULL, &renderQuad);
    SDL_DestroyTexture(textTexture);
    SDL_FreeSurface(textSurface);
}

SDL_Texture *loadTexture(SDL_Renderer *renderer, const char *path)
{
    SDL_Texture *newTexture = NULL;
    SDL_Surface *loadedSurface = IMG_Load(path);
    if (loadedSurface == NULL)
    {
        printf("Unable to load image %s! SDL_image Error: %s\n", path, IMG_GetError());
    }
    else
    {
        newTexture = SDL_CreateTextureFromSurface(renderer, loadedSurface);
        if (newTexture == NULL)
        {
            printf("Unable to create texture from %s! SDL Error: %s\n", path, SDL_GetError());
        }
        SDL_FreeSurface(loadedSurface);
    }
    return newTexture;
}

bool initializeSDL(SDL_Window **window, SDL_Renderer **renderer)
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return false;
    }

    if (TTF_Init() == -1)
    {
        printf("SDL_ttf could not initialize! TTF_Error: %s\n", TTF_GetError());
        return false;
    }

    *window = SDL_CreateWindow("VIP Cyclops", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (*window == NULL)
    {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return false;
    }

    *renderer = SDL_CreateRenderer(*window, -1, SDL_RENDERER_ACCELERATED);
    if (*renderer == NULL)
    {
        printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        return false;
    }

    SDL_SetRenderDrawColor(*renderer, 255, 255, 255, 1);
    int imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags))
    {
        printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
        return false;
    }

    return true;
}

void closeSDL(SDL_Window *window, SDL_Renderer *renderer, TTF_Font *font, SDL_Texture *mainTexture, SDL_Texture *coinTexture, SDL_Texture *backgroundTexture)
{
    SDL_DestroyTexture(mainTexture);
    SDL_DestroyTexture(coinTexture);
    SDL_DestroyTexture(backgroundTexture);
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    TTF_Quit();
    SDL_Quit();
}

bool loadMedia(TTF_Font **font, SDL_Texture **mainTexture, SDL_Texture **coinTexture, SDL_Renderer *renderer, const char *fontPath)
{
    *font = TTF_OpenFont(fontPath, 28);
    if (*font == NULL)
    {
        printf("Failed to load font! TTF_Error: %s\n", TTF_GetError());
        return false;
    }

    *mainTexture = loadTexture(renderer, CYCLOPS_IMAGE);
    if (*mainTexture == NULL)
    {
        printf("Failed to load main object texture!\n");
        return false;
    }

    *coinTexture = loadTexture(renderer, COIN_IMAGE);
    if (*coinTexture == NULL)
    {
        printf("Failed to load coin texture!\n");
        return false;
    }

    return true;
}

void resetGame(int *score, bool *gameOver, float *posX, float *posY, SDL_Rect *coinRect, SDL_Rect *blockers)
{
    *score = 0;
    *gameOver = false;
    CREATURE_SIZE = DEFAULT_CREATURE_SIZE;
    *posX = SCREEN_WIDTH / 2 - CREATURE_SIZE / 2;
    *posY = SCREEN_HEIGHT / 2 - CREATURE_SIZE / 2;

    coinRect->x = rand() % (SCREEN_WIDTH - COIN_SIZE);
    coinRect->y = rand() % (SCREEN_HEIGHT - COIN_SIZE);

    for (int i = 0; i < NUM_OF_BLOCKERS; i++)
    {
        do
        {
            blockers[i].x = rand() % (SCREEN_WIDTH - BLOCKER_SIZE);
            blockers[i].y = rand() % (SCREEN_HEIGHT - BLOCKER_SIZE);
        } while (SDL_HasIntersection(&blockers[i], &*coinRect) || SDL_HasIntersection(&blockers[i], &(SDL_Rect){(int)*posX, (int)*posY, CREATURE_SIZE, CREATURE_SIZE}));
    }
}

bool checkBlockerCollisions(SDL_Rect *rect, SDL_Rect *blockers)
{
    for (int i = 0; i < NUM_OF_BLOCKERS; i++)
    {
        if (SDL_HasIntersection(&*rect, &blockers[i]))
        {
            return true;
        }
    }
    return false;
}

void updatePlayerSize(SDL_Rect *playerRect, float *posX, float *posY, int newSize)
{
    playerRect->w = newSize;
    playerRect->h = newSize;
    *posX -= (newSize - playerRect->w) / 2;
    *posY -= (newSize - playerRect->h) / 2;
    playerRect->x = (int)*posX;
    playerRect->y = (int)*posY;
}

void generateBlockers(SDL_Rect *blockers, int numBlockers, SDL_Rect *playerRect, SDL_Rect *coinRect)
{
    for (int i = 0; i < numBlockers; i++)
    {
        do
        {
            blockers[i].x = rand() % (SCREEN_WIDTH - BLOCKER_SIZE);
            blockers[i].y = rand() % (SCREEN_HEIGHT - BLOCKER_SIZE);
            blockers[i].w = 15 + rand() % 41;
            blockers[i].h = 15 + rand() % 41;
        } while (SDL_HasIntersection(&blockers[i], playerRect) || SDL_HasIntersection(&blockers[i], coinRect));
    }
}