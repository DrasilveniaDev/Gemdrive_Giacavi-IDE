#include <stdio.h>
#include <SDL3/SDL.h>

#include <stddef.h>
#include "LCT_decode.h"

SDL_Texture* PackToTexture_BGRA(SDL_Renderer* mRen, const unsigned char* raw, const size_t raw_size, int width, int height){
    if(!mRen || !raw || width < 1 || height < 1) return NULL;
    size_t raw_size_Must = (size_t)width * 4 * height;
    if(raw_size_Must != raw_size) return NULL;

    const int ByteW = width * 4;
    SDL_Texture* mTex = SDL_CreateTexture(mRen, SDL_PIXELFORMAT_BGRA8888, SDL_TEXTUREACCESS_STATIC, width, height);
    if(!mTex) return NULL;

    if(SDL_UpdateTexture(mTex, NULL, (const void*)raw, ByteW) != 0) {
        SDL_DestroyTexture(mTex);
        return NULL;
    }
    SDL_SetTextureBlendMode(mTex, SDL_BLENDMODE_BLEND);

    return mTex;
}

SDL_Texture* CTexture_root(SDL_Renderer* mRen, const char* root){
    imageLCT_C mImagePack = importLCT_root(root);
    if(mImagePack.raw != NULL){
        SDL_Texture* mrTex = PackToTexture_BGRA(mRen, mImagePack.raw, mImagePack.raw_size, mImagePack.da, mImagePack.de);
        free_CPP(mImagePack.raw);
        return mrTex;
    }
    return NULL;
}

int main() {
    SDL_Window* win = NULL;
    SDL_Renderer* ren = NULL;

    if(SDL_Init(SDL_INIT_VIDEO) < 0) return 1;

    win = SDL_CreateWindow("Example", 520, 360, 0);
    if(win == NULL){
        SDL_Quit();
        return 1;
    }

    ren = SDL_CreateRenderer(win, NULL);
    if(ren == NULL){
        SDL_DestroyWindow(win);
        SDL_Quit();
        return 1;
    }

    int quit = 0;
    SDL_Event event;
    SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);

    /* Images */
    SDL_Texture* IMG_Example = CTexture_root(ren, "BMP_Vault.lct");

    while(quit == 0){
        while(SDL_PollEvent(&event)){
            switch(event.type){
                case SDL_EVENT_QUIT:
                    quit++;
                    break;
            }
        }
        SDL_RenderClear(ren);

        //Write the graphic project in this line

        SDL_RenderPresent(ren);
        SDL_Delay(33);
    }
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();

    return 0;

}
