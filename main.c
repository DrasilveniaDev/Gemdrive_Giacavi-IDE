#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>

#include <wchar.h>
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
        free(mPack.raw);
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

typedef struct font_C{
    unsigned char* raw;
    size_t raw_size;
    int fullW;
    int fullH;
    int charW;
    int charH;
    int ofsChar;
    int limChar;
} font_C;

void sFont(SDL_Renderer* mRen, SDL_Texture* mTex, font_C mFontDesc, float x, float y, float percent){
    SDL_FRect mRect = {x, y, (float)mFontDesc.charW * percent, (float)mFontDesc.charH * percent};
    SDL_RenderTexture(mRen, mTex, NULL, &mRect);
}

font_C importFont_root(const char* root, int mCharW, int mCharH, int mOffset, int CParam){
    font_C mFont = (font_C){.raw = NULL, .raw_size = 0, .ofsChar = mOffset};

    struct imageLCT_C mPackFT = importLCT_root(root);
    if(mPackFT.raw == NULL || mPackFT.C == 0 || mPackFT.raw_size == 0){
        fprintf(stderr, "importFont: Unvalid Image\n");
        return mFont;
    }
    if((int)mPackFT.width % mCharW != 0 || (int)mPackFT.height % mCharH != 0){
        fprintf(stderr, "importFont: Looks like the Static Charbox size or the F.Template size is incorrect\nIt has an extra size of Width: %d, Height: %d, that must be fixed\n", (int)mPackFT.width % mCharW, (int)mPackFT.height % mCharH);
        return mFont;
    }
    mFont.raw_size = (size_t)(mPackFT.width * mPackFT.height);
    if(mPackFT.raw_size / mPackFT.C != mFont.raw_size){
        fprintf(stderr, "importFont: image is not aligned\n");
        return mFont;
    }

    mFont.raw = (unsigned char*)malloc(mFont.raw_size);
    if(mFont.raw == NULL){
        fprintf(stderr, "importFont: Font full raw not allocated\n");
        return mFont;
    }

    int ValMixer = 0;
    for(int B = 0; B < mFont.raw_size; B++){
        switch(CParam){
            case 0:
                for(int C = 0; C < 3; C++){
                    ValMixer += (int)mPackFT.raw[B * mPackFT.C + C];
                }
                ValMixer /= 3;
                if(ValMixer > 255){
                    ValMixer = 255;
                }else if(ValMixer < 0){
                    ValMixer = 0;
                }
                break;
            case 1: // Blue
            case 2: // Green
            case 3: // Red
                ValMixer = (int)mPackFT.raw[B * mPackFT.C + (CParam - 1)];
                break;
            default:
                fprintf(stderr, "importFont: Unvalid Exclusive Color parameter\n");
                return mFont;
        }
        mFont.raw[B] = (unsigned char)ValMixer;
        ValMixer = 0;
    }
    if(mPackFT.raw != NULL) free(mFont.raw);

    mFont.fullW = (int)mPackFT.width;
    mFont.fullH = (int)mPackFT.height;
    mFont.charW = mCharW;
    mFont.charH = mCharH;
    mFont.ofsChar = mOffset;

    int charCantity = ((int)mPackFT.width / mCharW) * ((int)mPackFT.height / mCharH);
    mFont.limChar = mOffset + charCantity;

    return mFont;
}

struct imageLCT_C rawCharFromFont(font_C mFont, char mCharANSI, uint32_t lowC, uint32_t highC){ // Order is ARGB for the 2 last color arguments
    char mcChar = mCharANSI;
    if((unsigned char)mCharANSI < mFont.ofsChar)mcChar = (char)mFont.ofsChar;
    if(mCharANSI > mFont.limChar)mcChar = (char)mFont.limChar;

    struct imageLCT_C mCharRP;
    mCharRP.raw = NULL; 
    mCharRP.raw_size = 0; 
    mCharRP.C = 0;
    if(mFont.fullW <= 0 || mFont.fullH <= 0){
        fprintf(stderr, "raw C->Font: The full area size is 0\n");
        return mCharRP;
    }else if(mFont.charW <= 0 || mFont.charH <= 0){
        fprintf(stderr, "raw C->Font: The character area size is 0\n");
        return mCharRP;
    }else if(mFont.raw == NULL || mFont.raw_size == 0){
        fprintf(stderr, "raw C->Font: The Font Template is corrupted, empty or it was not created for some reason\n");
        return mCharRP;
    }

    mCharRP.raw_size = (mFont.charW * 4 * mFont.charH);
    mCharRP.raw = (unsigned char*)malloc(mCharRP.raw_size);
    if(mCharRP.raw == NULL){
        fprintf(stderr, "raw C->Font: Raw not allocated\n");
        return mCharRP;
    }
    mCharRP.width = mFont.charW;
    mCharRP.height = mFont.charH;

    int difC[4] = {0, 0, 0, 0};
    for(int C = 0; C < 4; C++){
        difC[C] = (int)((highC >> (C * 8)) & 0xFF) - (int)((lowC >> (C * 8)) & 0xFF);
        if(difC[C] > 255 || difC[C] < -255){
            difC[C] = 255 - 255 * (difC[C] < 0) * 2;
        }
    }
    unsigned char interC[4] = {0, 0, 0, 0};

    const int charTX = mFont.fullW / mFont.charW;
    int lvChar = (unsigned char)mcChar - mFont.ofsChar;

    for(int yCnt = 0; yCnt < mFont.charH; yCnt++){
        for(int xCnt = 0; xCnt < mFont.charW; xCnt++){
            int Xaxis = xCnt + (lvChar % charTX) * mFont.charW;
            int Yaxis = yCnt + (lvChar / charTX) * mFont.charH;

            if(Xaxis < 0 || Yaxis < 0 || Xaxis >= mFont.fullW || Yaxis >= mFont.fullH){
                fprintf(stderr, "We detect that the function is getting more than the expected.\n  Size is W %d, H %d.\n  Coordinates are under or overposing X %d, Y %d.\n", mFont.fullW, mFont.fullH, Xaxis, Yaxis);
                SDL_Delay(4000);
                return mCharRP;
            }

            for(int C = 0; C < 4; C++){
                interC[C] = (float)((lowC >> (C * 8)) & 0xFF);
                interC[C] += (float)difC[C] * ((float)mFont.raw[Xaxis + (Yaxis * mFont.fullW)] / 255.0f);
            }
            for(int C = 0; C < 4; C++){
                mCharRP.raw[C + (xCnt + (yCnt * mCharRP.width)) * 4] = interC[C];
            }
        }
    }
    mCharRP.C = 4;
    return mCharRP;
}

SDL_Texture* textureLCT_local(SDL_Renderer* mRen, struct imageLCT_C mPack){
    if(mPack.raw == NULL || mPack.raw_size == 0 || mPack.C == 0) return NULL;

    SDL_Surface* tr_sur = NULL;
    SDL_Texture* tr_tex = NULL;

    const int siep_pitch = mPack.width * 4;

    tr_sur = SDL_CreateSurfaceFrom((int)mPack.width, (int)mPack.height, SDL_PIXELFORMAT_BGRA32, mPack.raw, siep_pitch);
    if(tr_sur == NULL){
        fprintf(stderr, "Tex load loc: Surface Failed. %s\n", SDL_GetError());
        free(mPack.raw);
        return NULL;
    }

    tr_tex = SDL_CreateTextureFromSurface(mRen, tr_sur);
    if(tr_tex == NULL){
        fprintf(stderr, "Tex load loc: Texture Failed. %s\n", SDL_GetError());
    }
    free(mPack.raw);
    SDL_DestroySurface(tr_sur);
    return tr_tex;
}

int main(){
    SDL_Window* win = NULL;
    SDL_Renderer* ren = NULL;

    if(!SDL_Init(SDL_INIT_VIDEO)){
        fprintf(stderr, "SDL no initialized. %s\n", SDL_GetError());
        SDL_Delay(1500);
        return 1;
    }

    win = SDL_CreateWindow("Hello SDL", 1024, 680, 0);
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
    SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);

    /* Fonts */
    printf("Loading Font... ");
    SDL_Delay(2000);
    font_C FNT_PremierClassic = importFont_root("font/fontN-Wwes-12-20 Premier_Classic.lct", 12, 20, 0x20, 2);
    printf("DONE\nLoading Character Raw... ");
    SDL_Delay(2000);
    struct imageLCT_C IMGP_CharD = rawCharFromFont(FNT_PremierClassic, 'A', 0x00000000, 0xFF96CF83);
    printf("DONE\nCreating Texture... ");
    SDL_Delay(2000);
    SDL_Texture* IMG_CharD = NULL;
    if(IMGP_CharD.C != 0) IMG_CharD = textureLCT_local(ren, IMGP_CharD);
    printf("DONE\nInitialize Work\n");
    SDL_Delay(1000);

    while(quit == 0){
        while(SDL_PollEvent(&event)){
            switch(event.type){
                case SDL_EVENT_QUIT:
                    quit = 1;
                    break;
            }
        }
        SDL_RenderClear(ren);

        if(IMG_CharD != NULL) sFont(ren, IMG_CharD, FNT_PremierClassic, 100.0f, 100.0f, 1.0f);

        SDL_RenderPresent(ren);
        SDL_Delay(16);
    }

    /* Images End */
    SDL_DestroyTexture(IMG_CharD);
    if(IMGP_CharD.raw != NULL && IMGP_CharD.C != 4) free(IMGP_CharD.raw);

    free(FNT_PremierClassic.raw);

    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();

    return 0;

}
