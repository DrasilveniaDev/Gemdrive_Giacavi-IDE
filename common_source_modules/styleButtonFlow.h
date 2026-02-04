#ifndef STYLE_BUTTON_FLOW_SDL3
#define STYLE_BUTTON_FLOW_SDL3

#include <SDL3/SDL.h>
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C"{
#endif

extern void createButtonFlow_tex(SDL_Renderer* mRen, int ChaC, uint32_t *mFlowCFill, size_t mFCF_size, uint32_t *mFlowCBord, size_t mFCB_size, float posX, float posY, float miW, float miH, float bordS);

#ifdef __cplusplus
}
#endif

#endif