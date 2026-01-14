#include <stdio.h>
#include <stdlib.h>
#include <SDL3/SDL.h>
#include "LCT_decodeC.h"

SDL_Texture* textureLCT_root(SDL_Renderer* mRen, const char* root){
    struct imageLCT_C mPack = importLCT_root(root);
    if(mPack.raw == NULL || mPack.raw_size == 0 || mPack.C == 0) return NULL;

    SDL_Surface* tr_sur = NULL;
    SDL_Texture* tr_tex = NULL;

    const int siep_pitch = mPack.width * 4;

    tr_sur = SDL_CreateSurfaceFrom((int)mPack.width, (int)mPack.height, SDL_PIXELFORMAT_BGRA32, mPack.raw, siep_pitch);
    if(tr_sur == NULL){
        fprintf(stderr, "Tex load: Surface Failed. %s\n", SDL_GetError());
        return NULL;
    }

    tr_tex = SDL_CreateTextureFromSurface(mRen, tr_sur);
    if(tr_tex == NULL){
        fprintf(stderr, "Tex load: Texture Failed. %s\n", SDL_GetError());
    }
    free(mPack.raw);
    SDL_DestroySurface(tr_sur);
    return tr_tex;
}

void sTexture(SDL_Renderer* mRen, SDL_Texture* mTex, float xa, float xb, float ya, float yb){
    SDL_FRect mRect = {xa, ya, xb - xa, yb - ya};
    SDL_RenderTexture(mRen, mTex, NULL, &mRect);
}

int main(){
    SDL_Window* win = NULL;
    SDL_Renderer* ren = NULL;

    if(!SDL_Init(SDL_INIT_VIDEO)){
        fprintf(stderr, "SDL no initialized. %s\n", SDL_GetError());
        SDL_Delay(1500);
        return 1;
    }

    win = SDL_CreateWindow("Hello SDL", 800, 600, 0);
    if(win == NULL){
        fprintf(stderr, "SDL Window no created. %s\n", SDL_GetError());
        SDL_Delay(1500);
        SDL_Quit();
        return 1;
    }

    ren = SDL_CreateRenderer(win, NULL);
    if(ren == NULL){
        fprintf(stderr, "SDL Renderer no created. %s\n", SDL_GetError());
        SDL_DestroyWindow(win);
        SDL_Quit();
        return 1;
    }

    int quit = 0;
    SDL_Event event;
    SDL_SetRenderDrawColor(ren, 12, 64, 30, 255);

    /* Images. On debugging */
    SDL_Texture* IMG_Example = textureLCT_root(ren, "bmp_vault.lct");
    
    while(quit == 0){
        while(SDL_PollEvent(&event)){
            switch(event.type){
                case SDL_EVENT_QUIT:
                    quit = 1;
                    break;
            }
        }
        SDL_RenderClear(ren);

        sTexture(ren, IMG_Example, 272.0f, 528.0f, 172.0f, 428.0f);

        SDL_RenderPresent(ren);
        SDL_Delay(33);
    }

    /* Images End */
    SDL_DestroyTexture(IMG_Example);

    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();

    return 0;
}
