#ifdef _WIN32
    #include <windows.h>
#endif

#include <iostream>
#include <SDL3/SDL.h>
#include <string>
#include <vector>
#include <cstdint>

extern "C"{
    #include "LCT_decodeC.h"
    #include "unicode_kit.h"
}

// Text Codec Tools
std::string UTF8_to_win1252_SS(const std::string& mSString){
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

// Image & Font Tools
struct imgPack{
    std::vector<unsigned char> raw;
    uint32_t width;
    uint32_t height;
    int C;
};
struct fontPack{ // Monochromatic by default
    std::vector<unsigned char> raw;
    uint32_t fullW;
    uint32_t fullH;
    uint32_t charW;
    uint32_t charH;
    int valS;
    int valE;
};

imgPack importLCT_root_CPP(const char* root){
    struct imageLCT_C mCPack = importLCT_root(root);
    imgPack mPack;
    if(mCPack.raw != nullptr) mPack.raw.assign(mCPack.raw, mCPack.raw + mCPack.raw_size);
    mPack.width = mCPack.width;
    mPack.height = mCPack.height;
    mPack.C = mCPack.C;

    free(mCPack.raw);
    return mPack;
}
fontPack importFont_root(const char* root, uint32_t SCharW, uint32_t SCharH, int SvalS, int SvalE, int CParam){
    imgPack mipFontTemp = importLCT_root_CPP(root);
    fontPack mfpFont;
    mfpFont.fullW = mipFontTemp.width;
    mfpFont.fullH = mipFontTemp.height;
    mfpFont.charW = 0;
    mfpFont.charH = 0;
    mfpFont.valS = 0;
    mfpFont.valE = 0;

    if(mipFontTemp.raw.empty() || mipFontTemp.raw.size() != static_cast<size_t>(mipFontTemp.width * (uint32_t)mipFontTemp.C * mipFontTemp.height)){
        std::cerr << "The raw is empty or doesn't follow the expected size (importFont_root / err 0)" << std::endl;
        return mfpFont;
    }
    if(SvalE <= SvalS){
        std::cerr << "Char codec start & end value inserted an unvalid limit (importFont_root / err 1)" << std::endl;
        return mfpFont;
    }
    if(mfpFont.fullW % SCharW != 0 || mfpFont.fullH % SCharH != 0){
        std::cerr << "Charbox size of the font template inserted it's not correct or your template is not aligned. There's an offset of (W: " << mfpFont.fullW % SCharW << ", H: " << mfpFont.fullH % SCharH << ") on the template" << std::endl;
        return mfpFont;
    }
    mfpFont.charW = SCharW;
    mfpFont.charH = SCharH;

    mfpFont.raw.clear();
    for(int PixC = 0; PixC < mipFontTemp.raw.size(); PixC += mipFontTemp.C){
        float CVComb = 0.0f;
        switch(CParam){
            case 0:
                for(int C = 0; C < 3; C++){
                    CVComb += static_cast<float>(mipFontTemp.raw[PixC + C]);
                }
                CVComb /= 3;
                CVComb += 0.5f;
                if(CVComb < 0.0f) CVComb = 0.0f;
                if(CVComb > 255.0f) CVComb = 255.0f;
                mfpFont.raw.push_back(static_cast<unsigned char>(CVComb));
                break;
            case 1:
            case 2:
            case 3:
                mfpFont.raw.push_back(mipFontTemp.raw[(CParam - 1) + PixC]);
                break;
            default:
                std::cerr << "Unvalid Exclusive Color Filter Parameter (importFont_root / err 2)" << std::endl;
                return mfpFont;
        }
    }

    if(static_cast<uint32_t>(mfpFont.raw.size()) != mfpFont.fullW * mfpFont.fullH){
        std::cerr << "Font raw size looks that it's not monochromatic or incorrect (importFont_root / err 3)" << std::endl;
    }

    // Character Box Predicted Cantity
    uint32_t CBPC = (mfpFont.fullW / mfpFont.charW) * (mfpFont.fullH / mfpFont.charH);
    if(SvalE - SvalS != static_cast<int>(CBPC)) std::cout << "Characters needed according to Start & End parameters inserted, calculated with an arithmetic Charbox Cantity Prediction doesn't match to your template. Watch if there's something wrong with it\nCalculation:\n  Prediction: " << CBPC << "\n  Result: " << SvalE - SvalS << std::endl;

    return mfpFont;
}
SDL_Texture* textureImage(SDL_Renderer* mRen, const imgPack& mipImage){
    if(mipImage.raw.empty() || mipImage.C == 0){
        return nullptr;
    }

    const int mipX_pitch = mipImage.width * mipImage.C;
    SDL_Surface* mtrSur = nullptr;
    if(mipImage.C == 4){
        mtrSur = SDL_CreateSurfaceFrom((int)mipImage.width, (int)mipImage.height, SDL_PIXELFORMAT_BGRA32, mipImage.raw.data(), mipX_pitch);
    }else if(mipImage.C == 3){
        mtrSur = SDL_CreateSurfaceFrom((int)mipImage.width, (int)mipImage.height, SDL_PIXELFORMAT_BGR24, mipImage.raw.data(), mipX_pitch);
    }
    if(mtrSur == nullptr) return nullptr;

    SDL_Texture* mtrTex = SDL_CreateTextureFromSurface(mRen, mtrSur);
    SDL_DestroySurface(mtrSur);
    return mtrTex;
}

int main(){
    SDL_Window *win = nullptr;
    SDL_Renderer *ren = nullptr;

    if(!SDL_Init(SDL_INIT_VIDEO)){
        return 1;
    }

    win = SDL_CreateWindow("Gemdrive Giacavi 1.0a", 1024, 680, 0);
    if(win == nullptr){
        SDL_Quit();
        return 1;
    }

    ren = SDL_CreateRenderer(win, nullptr);
    if(ren == nullptr){
        SDL_DestroyWindow(win);
        SDL_Quit();
        return 1;
    }

    bool quit = false;
    SDL_Event event;
    SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);

    #ifdef _WIN32
        SetConsoleOutputCP(1252);
    #endif
    std::string TextTesty = "Gemdrive Softwares © 2026 - Spräche Deutsch";
    std::cout << UTF8_to_win1252_SS(TextTesty) << std::endl;

    while(!quit){
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