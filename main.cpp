#include <iostream>
#include <SDL3/SDL.h>
#include <string>
#include <vector>
#include <cstdint>
#include <utility>

extern "C"{
    #include "LCT_decodeC.h"
    #include "unicode_kit.h"
}

// Text Codec Tools
std::string UTF8_to_win1252_SS(const std::string& mSString){
    if(mSString.empty()) return "";

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
uint16_t UTF8_to_16b_oneChar(const std::string& mSChar){
    if(mSChar.empty()) return 0;

    U16char_ps mtcC16b = UTF8_to_16b(mSChar.c_str());
    if(mtcC16b.string == nullptr) return 0;

    uint16_t mC16b = mtcC16b.string[0];
    S16b_free(mtcC16b.string);
    return mC16b;
}
std::u16string UTF8_to_16b_SS(const std::string& mSString){
    if(mSString.empty()) return u"";

    U16char_ps mtS16b = UTF8_to_16b(mSString.c_str());
    if(mtS16b.string == nullptr) return u"";

    std::u16string mS16b(reinterpret_cast<char16_t*>(mtS16b.string), mtS16b.size);
    S16b_free(mtS16b.string);
    return mS16b;
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
    uint32_t CBAC = (SvalE - SvalS) + 1;
    if(CBAC != CBPC) std::cout << "Characters needed according to Start & End parameters inserted, calculated with an arithmetic Charbox Cantity Prediction doesn't match to your template. Watch if there's something wrong with it\nCalculation:\n  Prediction: " << CBPC << "\n  Result: " << CBAC << std::endl;
    mfpFont.valS = SvalS;
    mfpFont.valE = SvalE;

    return mfpFont;
}
SDL_Texture* textureImage(SDL_Renderer* mRen, imgPack& mipImage){
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

imgPack customizeCharFF(const fontPack& mFont, uint16_t sChar, uint32_t lColor, uint32_t hColor){
    uint16_t mfxChar = sChar;
    mfxChar = static_cast<uint16_t>((int)mfxChar < mFont.valS ? mFont.valS & 0xFFFF : ((int)mfxChar > mFont.valE ? mFont.valE & 0xFFFF : mfxChar));
    int mf0Char = static_cast<int>(mfxChar) - mFont.valS;

    int fontCBFC = static_cast<int>(mFont.fullW / mFont.charW);
    std::pair<int,int> charUbication = {mf0Char % fontCBFC, mf0Char / fontCBFC};

    imgPack mipBChar;
    mipBChar.C = 0;
    mipBChar.width = mFont.charW;
    mipBChar.height = mFont.charH;

    std::vector<float> CIDiference;
    for(int C = 0; C < 4; C++){
        int lSValC = (lColor >> (C << 3)) & 0xFF;
        int hSValC = (hColor >> (C << 3)) & 0xFF;
        CIDiference.push_back(static_cast<float>(hSValC - lSValC));
        CIDiference[C] = CIDiference[C] < -255.0f ? -255.0f : (CIDiference[C] > 255.0f ? 255.0f : CIDiference[C]);
    }
    if(CIDiference.size() != 4){
        std::cerr << "CIDiference list size is not 4. It resulted with a size of " << CIDiference.size() << " (customizeCharFF / err 0)" << std::endl;
        return mipBChar;
    }

    int ColInterpoled = 0;
    for(int YCcut = 0; YCcut < (int)mFont.charH; YCcut++){
        for(int XCcut = 0; XCcut < (int)mFont.charW; XCcut++){
            int svPixelCutI = XCcut + (charUbication.first * (int)mFont.charW) + (YCcut + (charUbication.second * (int)mFont.charH)) * (int)mFont.fullW;
            for(int C = 0; C < 4; C++){
                if(svPixelCutI < 0 || svPixelCutI >= static_cast<int>(mFont.raw.size())){
                    std::cout << "Result is going out the limits. That part will fill with a Semi-transparent Blue color (customizeCharFF / warn 0)" << std::endl;
                    mipBChar.raw.push_back(0xFF);
                    mipBChar.raw.push_back(0x00);
                    mipBChar.raw.push_back(0x00);
                    mipBChar.raw.push_back(0x80);
                    break;
                }

                ColInterpoled = static_cast<int>((lColor >> (C << 3)) & 0xFF) + static_cast<int>(CIDiference[C] * (static_cast<float>(mFont.raw[svPixelCutI]) / 255.0f));
                ColInterpoled = ColInterpoled < 0 ? 0 : (ColInterpoled > 255 ? 255 : ColInterpoled);
                mipBChar.raw.push_back(static_cast<unsigned char>(ColInterpoled & 0xFF));
            }
        }
    }

    if(static_cast<uint32_t>(mipBChar.raw.size()) != mipBChar.width * 4 * mipBChar.height){
        std::cout << "Raw size was not expected";
        if(mipBChar.raw.size() % 4 != 0) std::cout << " and color data it's not aligned into RGBA";
        std::cout << ", So it will fill with a Semi-transparent Blue color (customizeCharFF / warn 1)" << std::endl;
        switch(mipBChar.raw.size() % 4){
            case 1:
                mipBChar.raw.push_back(0x00);
            case 2:
                mipBChar.raw.push_back(0x00);
            case 3:
                mipBChar.raw.push_back(0x80);
        }
        while(mipBChar.raw.size() < static_cast<size_t>(mipBChar.width * 4 * mipBChar.height)){
            switch(mipBChar.raw.size() % 4){
                case 0:
                    mipBChar.raw.push_back(0xFF);
                case 1:
                    mipBChar.raw.push_back(0x00);
                case 2:
                    mipBChar.raw.push_back(0x00);
                case 3:
                    mipBChar.raw.push_back(0x80);
            }
        }
    }

    if(mipBChar.raw.size() > mipBChar.width * 4 * mipBChar.height){
        std::cout << "Raw size is major than expected (warn 2)" << std::endl;
        mipBChar.raw.resize(mipBChar.width * 4 * mipBChar.height);
    }

    mipBChar.C = 4;
    return mipBChar;
}

char getOnlyChar_8b(std::string mUnChar){
    return mUnChar.empty() ? 0 : mUnChar[0];
}
uint16_t getOnlyChar_16b(std::u16string mUnChar16){
    return static_cast<uint16_t>(mUnChar16.empty() ? 0 : mUnChar16[0]);
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

    // Fonts
    const fontPack FNT_Premier_Classic = importFont_root("font/fontN-Wwes-12-20 Premier_Classic.lct", 12, 20, 0x20, 0xFF, 2);

    /* This part is made for testing, and this code will be changed if we go more advanced */
    imgPack IMGP_CharCIP = customizeCharFF(FNT_Premier_Classic, static_cast<uint16_t>(getOnlyChar_8b(UTF8_to_win1252_SS("Å"))) & 0xFF, 0xFF90238A, 0xFFFFFFFF);
    SDL_Texture* TEX_CharDT = textureImage(ren, IMGP_CharCIP);
    SDL_FRect charRect = {0, 0, 12, 20};
    /* End of the Case */
    if(TEX_CharDT == nullptr) std::cout << "Texture not registered. Texturize failed" << std::endl;

    while(!quit){
        while(SDL_PollEvent(&event)){
            switch(event.type){
                case SDL_EVENT_QUIT:
                    quit = true;
                    break;
            }
        }
        SDL_RenderClear(ren);

        if(TEX_CharDT != nullptr) SDL_RenderTexture(ren, TEX_CharDT, nullptr, &charRect);

        SDL_RenderPresent(ren);
        SDL_Delay(33);
    }

    SDL_DestroyTexture(TEX_CharDT);

    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}