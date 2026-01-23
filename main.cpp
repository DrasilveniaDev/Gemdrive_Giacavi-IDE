#include <iostream>
#include <SDL3/SDL.h>

#include <string>

std::string UTF8_to_win1252_SS(std::string mSString){
    U16char_ps mS16b = UTF8_to_16b(mSString.c_str());
    if(mS16b.string == nullptr) return "";

    char* mtS8b = S16b_to_win1252(mS16b);
    S16b_free(mS16b.string);
    if(mtS8b == nullptr){
        return "";
    }

    std::string frString8b(mtS8b);
    S8b_free(mtS8b);
    return frString8b;
}

int main(){
    SDL_Window *win = NULL;
    SDL_Renderer *ren = NULL;

    if(!SDL_Init(SDL_INIT_VIDEO)){
        return 1;
    }

    win = SDL_CreateWindow("Gemdrive Giacavi 1.0", 1024, 680, 0);
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

    bool quit = false;

    SDL_Event event;
    SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
    while(quit == 0){
        while(SDL_PollEvent(&event)){
            switch(event.type){
                case SDL_EVENT_QUIT:
                    quit = true;
                    break;
            }
        }
        SDL_RenderClear(ren);

        SDL_RenderPresent(ren);
        SDL_Delay(33);
    }

    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}