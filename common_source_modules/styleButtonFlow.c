#include <SDL3/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include "styleButtonFlow.h"

void createButtonFlow_tex(SDL_Renderer* mRen, int ChaC, uint32_t *mFlowCFill, size_t mFCF_size, uint32_t *mFlowCBord, size_t mFCB_size, float posX, float posY, float miW, float miH, float bordS){
    if(ChaC != 3 && ChaC != 4) return;
    if(mFlowCFill == NULL || mFlowCBord == NULL || mFCF_size == 0 || mFCB_size == 0) return;
    if(miW < 1 || miH < 1){
        fprintf(stderr, "You don't think that it's more easy Put Width and Height positive?\nlook this:\n  Width: %d\n  Height: %d\n", miW, miH);
        return;
    }
    if(bordS < 0) return;

    unsigned char *mpc_Fill = (unsigned char*)malloc(mFCF_size * ChaC);
    if(mpc_Fill == NULL) return;
    for(int CS = 0; CS < (int)mFCF_size * ChaC; CS++){
        mpc_Fill[CS] = (unsigned char)((mFlowCFill[CS / ChaC] >> ((CS % ChaC) << 3)) & 0xFF);
    }

    unsigned char *mpc_Bord = NULL;
    if(bordS != 0){
        mpc_Bord = (unsigned char*)malloc(mFCB_size * ChaC);
        if(mpc_Bord == NULL){
            free(mpc_Fill); // Because mpc_Fill goes first and next mpc_Bord
            return;
        }

        for(int CS = 0; CS < (int)mFCB_size * ChaC; CS++){
            mpc_Bord[CS] = (unsigned char)((mFlowCBord[CS / ChaC] >> ((CS % ChaC) << 3)) & 0xFF);
        }
    }

    SDL_Surface *msur_Fill = NULL;
    SDL_Surface *msur_Bord = NULL;
    SDL_Texture *mtex_Fill = NULL;
    SDL_Texture *mtex_Bord = NULL;

    msur_Fill = SDL_CreateSurfaceFrom(1, (int)mFCF_size, (ChaC == 4) ? SDL_PIXELFORMAT_BGRA32 : SDL_PIXELFORMAT_BGR24, mpc_Fill, ChaC);
    if(msur_Fill == NULL){
        free(mpc_Fill);
        if(bordS != 0) free(mpc_Bord);
        return;
    }
    mtex_Fill = SDL_CreateTextureFromSurface(mRen, msur_Fill);
    SDL_DestroySurface(msur_Fill);
    if(mtex_Fill == NULL){
        free(mpc_Fill);
        if(bordS != 0) free(mpc_Bord);
        return;
    }

    SDL_FRect mRect = {posX - bordS, posY, miW + (bordS * 2), miH};
    if(bordS != 0){
        msur_Bord = SDL_CreateSurfaceFrom(1, (int)mFCB_size, (ChaC == 4) ? SDL_PIXELFORMAT_BGRA32 : SDL_PIXELFORMAT_BGR24, mpc_Bord, ChaC);
        if(msur_Bord == NULL){
            free(mpc_Fill);
            free(mpc_Bord);
            SDL_DestroyTexture(mtex_Fill);
            return;
        }
        mtex_Bord = SDL_CreateTextureFromSurface(mRen, msur_Bord);
        SDL_DestroySurface(msur_Bord);
        if(mtex_Bord == NULL){
            free(mpc_Fill);
            free(mpc_Bord);
            SDL_DestroyTexture(mtex_Fill);
            return;
        }

        SDL_RenderTexture(mRen, mtex_Bord, NULL, &mRect);
        SDL_DestroyTexture(mtex_Bord);
        mRect.y = posY - bordS;
        mRect.h = bordS;

        SDL_SetRenderDrawColor(mRen, mpc_Bord[2], mpc_Bord[1], mpc_Bord[0], (ChaC == 4) ? mpc_Bord[3] : 255);
        SDL_RenderFillRect(mRen, &mRect);

        mRect.y = posY + miH;
        int LastIndex = ChaC * (mFCB_size - 1);
        SDL_SetRenderDrawColor(mRen, mpc_Bord[LastIndex + 2], mpc_Bord[LastIndex + 1], mpc_Bord[LastIndex], (ChaC == 4) ? mpc_Bord[LastIndex + 3] : 255);

        SDL_RenderFillRect(mRen, &mRect);
        SDL_SetRenderDrawColor(mRen, 0, 0, 0, 255);
        free(mpc_Bord);
    }

    mRect = (SDL_FRect){posX, posY, miW, miH};
    SDL_RenderTexture(mRen, mtex_Fill, NULL, &mRect);

    SDL_DestroyTexture(mtex_Fill);
    free(mpc_Fill);
}