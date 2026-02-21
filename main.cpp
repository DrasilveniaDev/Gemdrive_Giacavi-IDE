#include <iostream>
#include <fstream>

#include <SDL3/SDL.h>
#include <string>
#include <vector>
#include <cstdint>
#include <utility>
#include <unordered_map>
#include <cmath>

extern "C"{
    #include "LCT_decodeC.h"
    #include "unicode_kit.h"
    #include "styleButtonFlow.h"
    #include "codeTokenType.h"
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
    if(mipImage.C == 3 || mipImage.C == 4){
        mtrSur = SDL_CreateSurfaceFrom((int)mipImage.width, (int)mipImage.height, (mipImage.C == 4) ? SDL_PIXELFORMAT_BGRA32 : SDL_PIXELFORMAT_BGR24, mipImage.raw.data(), mipX_pitch);
    }else{
        std::cerr << "Invalid Color cantity" << std::endl;
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

std::unordered_map<std::string, std::pair<uint32_t, uint32_t>> importDataList(std::string mRoot){
    std::unordered_map<std::string, std::pair<uint32_t, uint32_t>> mDataList;
    std::ifstream mfDL(mRoot);
    if(!mfDL.is_open()) return mDataList;

    std::string msDataList((std::istreambuf_iterator<char>(mfDL)), std::istreambuf_iterator<char>());
    mfDL.close();
    if(msDataList.empty()) return mDataList;

    struct DLObj_sty{
        std::string param;
        std::pair<uint32_t, uint32_t> style;
    };
    const std::vector<std::pair<std::string, int>> TTHexConv = {
        {"Aa", 10},
        {"Bb", 11},
        {"Cc", 12},
        {"Dd", 13},
        {"Ee", 14},
        {"Ff", 15}
    };
    DLObj_sty mopDL;
    size_t DLParseI = 0;

    while(DLParseI < msDataList.size()){
        char mCharParse = msDataList[DLParseI];
        while(mCharParse != '='){
            if((mCharParse >= 0x41 && mCharParse < 0x5B) || (mCharParse >= 0x61 && mCharParse < 0x7B) || mCharParse == '_') mopDL.param.push_back(mCharParse);
            if(++DLParseI < msDataList.size()){
                mCharParse = msDataList[DLParseI];
            }else{
                break;
            }
        }
        int tivHex = 0;
        while(mCharParse != 'x' && ++DLParseI < msDataList.size()){
            mCharParse = msDataList[DLParseI];
        }
        if(++DLParseI < msDataList.size()) mCharParse = msDataList[DLParseI];

        for(int CHC = 0; CHC < 8; CHC++){
            if(mCharParse == ',') break;
            bool LHexF = false;
            for(int i = 0; i < 6; i++){
                if(TTHexConv[i].first[0] == mCharParse || TTHexConv[i].first[1] == mCharParse){
                    LHexF = true;
                    tivHex = TTHexConv[i].second;
                    break;
                }
            }
            if(!LHexF) tivHex = mCharParse >= 0x30 && mCharParse < 0x3A ? (static_cast<int>(mCharParse) & 0xF) : 0;
            tivHex = tivHex < 0 ? 0 : (tivHex > 15 ? 15 : tivHex);

            mopDL.style.first |= tivHex << ((7 - CHC) << 2);
            if(++DLParseI < msDataList.size()){
                mCharParse = msDataList[DLParseI];
            } else {
                break;
            }
            tivHex = 0;
        }
        while(mCharParse != 'x' && ++DLParseI < msDataList.size()){
            mCharParse = msDataList[DLParseI];
        }
        if(++DLParseI < msDataList.size()) mCharParse = msDataList[DLParseI];

        if(tivHex != 0) tivHex = 0;
        for(int CHC = 0; CHC < 8; CHC++){
            if(mCharParse == ',' || mCharParse == '\n' || mCharParse == '\r') break;
            bool LHexF = false;
            for(int i = 0; i < 6; i++){
                if(TTHexConv[i].first[0] == mCharParse || TTHexConv[i].first[1] == mCharParse){
                    LHexF = true;
                    tivHex = TTHexConv[i].second;
                    break;
                }
            }
            if(!LHexF) tivHex = mCharParse >= 0x30 && mCharParse < 0x3A ? (static_cast<int>(mCharParse) & 0xF) : 0;
            tivHex = tivHex < 0 ? 0 : (tivHex > 15 ? 15 : tivHex);

            mopDL.style.second |= tivHex << ((7 - CHC) << 2);
            if(++DLParseI < msDataList.size()){
                mCharParse = msDataList[DLParseI];
            } else {
                break;
            }
            tivHex = 0;
        }
        mDataList[mopDL.param] = mopDL.style;
        mopDL.param.clear();
        mopDL.style = {0, 0};

        while(mCharParse == '\n' || mCharParse == '\r' || mCharParse == ' ' || mCharParse == '\t'){
            if(++DLParseI < msDataList.size()){
                mCharParse = msDataList[DLParseI];
            }else{
                break;
            }
        }
    }

    return mDataList;
}

int tabLSize = 4; // The tab size will be 4 by default. If this changes, will display another thing.

std::vector<int> categorizeCode_token_VEC(const std::u16string& mTCode_U16){
    int* mTokenCategoryString = categorizeCode_token(reinterpret_cast<const uint16_t*>(mTCode_U16.c_str()));
    if(mTokenCategoryString == nullptr) return {};

    std::vector<int> mpvTCS(mTokenCategoryString, mTokenCategoryString + mTCode_U16.size() + 1);
    free(mTokenCategoryString); 
    return mpvTCS;
}

// ArrayDText_UTF16(ren, FNT_Premier_Classic, importDataList("data/mainTheme.csv"), u"UTF-16 Example", 1024, 680, 0, 120, roadX, roadY, 1.0f);

void ArrayDText_UTF16(SDL_Renderer* mRen, const fontPack& mFont, std::unordered_map<std::string, std::pair<uint32_t, uint32_t>> dlTextStyle, const std::u16string& textFile_U16, int screen_w, int screen_h, int dx, int dy, int cx, int cy, float resizeDC){
    if(dlTextStyle.find("SText") == dlTextStyle.end()){
        dlTextStyle["SText"] = {0xFF000000, 0xFFFFFFFF};
        std::cout << "The data list doesn't have token \"SText\". It will be assigned with these colors by default {Black, White} (ArrayDText / warn 0)" << std::endl;
    }

    auto StyleAClass = dlTextStyle.end();
    std::pair<uint32_t, uint32_t> Style2C = {0,0};

    char16_t prevChar = 0;
    bool contLoop = true;
    SDL_Texture* mtexDChar = nullptr;

    if(screen_w <= dx || screen_h <= dy){
        std::cout << "Display will be hide because the Display point position is out of the screen visible zone. (ArrayDText / warn 1)\nNote that as it's hidden, texture proccess will be ignored." << std::endl;
        return;
    }

    std::vector<int> mtpr_textPaint = categorizeCode_token_VEC(textFile_U16);

    if(mtpr_textPaint.empty() || mtpr_textPaint.size() - 1 != textFile_U16.size()){
        std::cerr << "Paint Vector failed or it was not the same size as the text (ArrayDText / err 0)" << std::endl;
        return;
    }

    float ACD_x = static_cast<float>(dx);
    float ACD_y = static_cast<float>(dy);
    const std::pair<float,float> CDS = {static_cast<float>(mFont.charW) * resizeDC, static_cast<float>(mFont.charH) * resizeDC};

    imgPack mipDChar;
    bool takedSpChar = false;
    SDL_FRect mduCharRect = {0.0f, 0.0f, 0.0f, 0.0f};

    std::u16string DispCompressTF_U16; // This is the cutted part.
    std::vector<int> mtpr_textPaint_Comp;
    const std::pair<int,int> CCDC = {
        static_cast<int>(std::ceil((screen_w - dx) / CDS.first)),
        static_cast<int>(std::ceil((screen_h - dy) / CDS.second))
    };
    std::pair<int,int> CharUbic = {0,0};

    int CPaint_Comp = 0;
    for(const char16_t& mChar16_p : textFile_U16){
        switch(mChar16_p){
            case 0x000D:
            case 0x000A:
                if(CharUbic.second - cy >= 0 && CharUbic.second - cy < CCDC.second){
                    DispCompressTF_U16.push_back(mChar16_p);
                    mtpr_textPaint_Comp.push_back(mtpr_textPaint[CPaint_Comp]);
                }
                CharUbic.first = 0;
                if(mChar16_p == 0x000A) CharUbic.second++;
                break;
            default:
                if(CharUbic.first - cx >= 0 && CharUbic.first - cx < CCDC.first && CharUbic.second - cy >= 0 && CharUbic.second - cy < CCDC.second){
                    DispCompressTF_U16.push_back(mChar16_p);
                    mtpr_textPaint_Comp.push_back(mtpr_textPaint[CPaint_Comp]);
                }
                CharUbic.first += mChar16_p == 0x0009 ? ::tabLSize : 1;
                break;
        }
        CPaint_Comp++;
    }

    std::string styleIDECase = "SText";
    std::string styleIDECase_prev;
    CPaint_Comp = 0;
    for(const char16_t& mChar16 : DispCompressTF_U16){
        styleIDECase = get_tokenNameF_int(mtpr_textPaint_Comp[CPaint_Comp]);
        auto styleFindIter = dlTextStyle.find(styleIDECase);

        if(styleFindIter == dlTextStyle.end()){
            if(styleIDECase == "SPunctuationD") styleFindIter = dlTextStyle.find("SPunctuation");
            styleFindIter = styleFindIter != dlTextStyle.end() ? styleFindIter : dlTextStyle.find("SText");
        }
        if(styleFindIter == dlTextStyle.end()){
            std::cerr << "Looks like the *SText* style autoinclusion failed (ArrayDText / err 1)" << std::endl;
            if(mtexDChar != nullptr) SDL_DestroyTexture(mtexDChar);
            return;
        }
        Style2C = {styleFindIter->second.first, styleFindIter->second.second};

        switch(static_cast<uint16_t>(mChar16)){
            case 0x000D:
                SDL_SetRenderDrawColor(mRen, ((Style2C.first >> 16) & 0xFF), ((Style2C.first >> 8) & 0xFF), (Style2C.first & 0xFF), ((Style2C.first >> 24) & 0xFF));
                mduCharRect = {ACD_x, ACD_y, static_cast<float>(screen_w) - ACD_x, CDS.second};
                SDL_RenderFillRect(mRen, &mduCharRect);
                SDL_SetRenderDrawColor(mRen, 0, 0, 0, 255);

                ACD_x = static_cast<float>(dx);
                takedSpChar = true;
                break;
            case 0x000A:
                if(0x000D != prevChar){
                    SDL_SetRenderDrawColor(mRen, ((Style2C.first >> 16) & 0xFF), ((Style2C.first >> 8) & 0xFF), (Style2C.first & 0xFF), ((Style2C.first >> 24) & 0xFF));
                    mduCharRect = {ACD_x, ACD_y, static_cast<float>(screen_w) - ACD_x, CDS.second};
                    SDL_RenderFillRect(mRen, &mduCharRect);
                    SDL_SetRenderDrawColor(mRen, 0, 0, 0, 255);

                    ACD_x = static_cast<float>(dx);
                }
                ACD_y += CDS.second;
                takedSpChar = true;
                break;
            case 0x0009:
                SDL_SetRenderDrawColor(mRen, ((Style2C.first >> 16) & 0xFF), ((Style2C.first >> 8) & 0xFF), (Style2C.first & 0xFF), ((Style2C.first >> 24) & 0xFF));
                mduCharRect = {ACD_x, ACD_y, CDS.first * ::tabLSize, CDS.second};
                SDL_RenderFillRect(mRen, &mduCharRect);
                SDL_SetRenderDrawColor(mRen, 0, 0, 0, 255);

                ACD_x += CDS.first * ::tabLSize;
                takedSpChar = true;
                break;
            default:
                if(contLoop || mChar16 != prevChar || styleIDECase != styleIDECase_prev){
                    mipDChar = customizeCharFF(mFont, static_cast<uint16_t>(mChar16), Style2C.first, Style2C.second);

                    if(mtexDChar != nullptr) SDL_DestroyTexture(mtexDChar);
                    mtexDChar = textureImage(mRen, mipDChar);
                }
                break;
        }
        if(!takedSpChar){
            mduCharRect = {ACD_x, ACD_y, CDS.first, CDS.second};
            SDL_RenderTexture(mRen, mtexDChar, nullptr, &mduCharRect);
            ACD_x += CDS.first;
        }

        if(contLoop) contLoop = false;
        prevChar = mChar16;
        styleIDECase_prev = styleIDECase;
        if(takedSpChar) takedSpChar = false;
        CPaint_Comp++;
    }

    SDL_SetRenderDrawColor(mRen, ((Style2C.first >> 16) & 0xFF), ((Style2C.first >> 8) & 0xFF), (Style2C.first & 0xFF), ((Style2C.first >> 24) & 0xFF));
    if(prevChar != 0x000A){
        mduCharRect = {ACD_x, ACD_y, static_cast<float>(screen_w) - ACD_x, CDS.second};
        SDL_RenderFillRect(mRen, &mduCharRect);

        ACD_x = static_cast<float>(dx);
        ACD_y += CDS.second;
    }
    if(ACD_y < static_cast<float>(screen_h)){
        mduCharRect = {ACD_x, ACD_y, static_cast<float>(screen_w) - ACD_x, static_cast<float>(screen_h) - ACD_y};
        SDL_RenderFillRect(mRen, &mduCharRect);
    }
    SDL_SetRenderDrawColor(mRen, 0, 0, 0, 255);
    if(mtexDChar != nullptr) SDL_DestroyTexture(mtexDChar);
}

void createButtonFlow_tex_X(SDL_Renderer* mRen, int ChaC, std::vector<uint32_t> mFlowCFill, size_t mFCF_size, std::vector<uint32_t>mFlowCBord, size_t mFCB_size, float posX, float posY, float miW, float miH, float bordS){
    createButtonFlow_tex(mRen, ChaC, mFlowCFill.data(), mFCF_size, mFlowCBord.data(), mFCB_size, posX, posY, miW, miH, bordS);
}

struct SPFontPack{ // Smart Portable Font Pack
    std::string name;
    std::pair<int,int> charBS; // Optional Use (Charbox Size)
    float charDS; // Optional Use (Char Display Size)
    std::unordered_map<std::string, fontPack> fontPackCPD; // font pack Code Page Display
};

SPFontPack loadResource_SPFont(std::string mRoot, float ivDSize){
    const std::string fs_Err0 = "Smart Font Resource file was deleted, cleared or doesn't exist.\nThe type of file is \".fnt.csv\" for know (loadResource_SPFont / err 0)";
    const std::string fs_Err2 = "Reader ended too early (loadResource_SPFont / err 2)";
    const std::string fs_Err4_1 = "Numbers inserted on strings aren't numbers (loadResource_SPFont / err 4.1)";

    SPFontPack mSPFont;
    std::ifstream mSPFResource_F("font/" + mRoot);
    if(!mSPFResource_F.is_open()){
        std::cerr << fs_Err0 << std::endl;
        return mSPFont;
    }

    std::string mSPFResource_S((std::istreambuf_iterator<char>(mSPFResource_F)), std::ifstreambuf_iterator<char>());
    mSPFResource_F.close();
    if(mSPFResource_S.empty()){
        std::cerr << fs_Err0 << std::endl;
        return mSPFont;
    }
    if(mRoot.empty()){
        std::cerr << "Root is not inserted on loadResource_SPFont function (loadResource_SPFont / err 1)" << std::endl;
        return mSPFont;
    }

    // Remember: mSPFResource_S; S means String
    int C = 0;
    int S = 0;
    char dsi_Char = mSPFResource_S[C];
    while(dsi_Char != '\r' && dsi_Char != '\n' && dsi_Char != '`'){
        C++;
        if(C >= mSPFResource_S.size()){
            std::cout << fs_Err2 << std::endl;
            return mSPFont;
        }
    }
    mSPFont.name = mSPFResource_S.substr(S, C);

    bool SCPass = false;
    if(mSPFont.name[mSPFont.name.size() - 1] == ' '){
        for(size_t CHC = mSPFont.name.size(); CHC > 0; CHC--){
            if(mSPFont.name[CHC - 1] != ' '){
                SCPass = true;
                mSPFont.name = mSPFont.name.substr(0, CHC);
                break;
            }
        }
        if(!SCPass){
            std::cerr << "Name is all spaced (loadResource_SPFont / err 3.0)" << std::endl;
            return mSPFont;
        }
    }

    while(mSPFResource_S[C] != '\r' || mSPFResource_S[C] != '\n'){
        C++;
        if(C >= mSPFResource_S.size()){
            std::cerr << fs_Err2 << std::endl;
            return mSPFont;
        }
    }
    while(mSPFResource_S[C] == '\r' || mSPFResource_S[C] == '\n'){
        C += mSPFResource_S[C] == '\r' ? 2 : 1;
        if(C >= mSPFResource_S.size()){
            std::cerr << fs_Err2 << std::endl; 
            return mSPFont;
        }
    }

    std::string stoi_NWidth;
    std::string stoi_NHeight;

    S = C;
    while(mSPFResource_S[C] != ' '){
        C++;
        if(C >= mSPFResource_S.size()){
            std::cerr << fs_Err2 << std::endl;
            return mSPFont;
        }
        if(mSPFResource_S[C] == '\r' || mSPFResource_S[C] == '\n'){
            std::cerr << "Not enough Numbers. It needs 2 (loadResource_SPFont / err 3.1)" << std::endl;
            return mSPFont;
        }
    }
    stoi_NWidth = mSPFResource_S.substr(S, C - S);
    do{
        S = ++C;
    }while(mSPFResource_S[C] == ' ');

    SCPass = false;
    S = C;
    while(true){
        switch(mSPFResource_S[C]){
            case ' ':
            case '\r':
            case '\n':
            case '`':
                SCPass = true;
                break;
            default:
                C++;
                if(C >= mSPFResource_S.size()){
                    std::cerr << fs_Err2 << std::endl;
                    return mSPFont;
                }
                break;
        }
        if(SCPass) break;
    }
    stoi_NHeight = mSPFResource_S.substr(S, C - S);

    if(stoi_NWidth.empty() || stoi_NHeight.empty()){
        std::cerr << "Number is all spaced or empty (loadResource_SPFont / err 4.0)" << std::endl;
        return mSPFont;
    }

    std::pair<int,int> sCDimensions = {0, 0};
    try{
        sCDimensions = {std::stoi(stoi_NWidth), std::stoi(stoi_NHeight)};
    }catch(...){
        std::cerr << fs_Err4_1 << std::endl;
        return mSPFont;
    }

    if(sCDimensions.first < 2 || sCDimensions.second < 2){
        std::cerr << "Character Size readed is too small (loadResource_SPFont / err 4.2)" << std::endl;
        return mSPFont;
    }

    mSPFont.charBS = sCDimensions;

    while(mSPFResource_S[C] != '\r' || mSPFResource_S[C] != '\n'){
        C++;
        if(C >= mSPFResource_S.size()){
            std::cerr << fs_Err2 << std::endl;
            return mSPFont;
        }
    }
    while(mSPFResource_S[C] == '\r' || mSPFResource_S[C] == '\n'){
        C += mSPFResource_S[C] == '\r' ? 2 : 1;
        if(C >= mSPFResource_S.size()){
            std::cerr << fs_Err2 << std::endl; 
            return mSPFont;
        }
    }

    std::string tNameFP;
    struct FPCP_CutFont{
        int CharCX;
        int CharCY;
        int SCD;
        char CCX;
    };
    FPCP_CutFont fxCutFont;

    while(C < mSPFResource_S.size()){
        S = C;
        while(mSPFResource_S[C] != ' '){
            C++;
            if(C >= mSPFResource_S.size()){
                std::cerr << fs_Err2 << std::endl;
                return mSPFont;
            }
        }

        tNameFP = mSPFResource_S.substr(S, C - S);
        C++;
        if(mSPFResource_S[C] == '"'){
            S = ++C;
            while(mSPFResource_S[C] != '"'){
                C++;
                if(C >= mSPFResource_S.size()){
                    std::cerr << fs_Err2 << std::endl;
                    return mSPFont;
                }
            }
        }else{
            S = C;
            while(mSPFResource_S[C] != ' '){
                C++;
                if(C >= mSPFResource_S.size()){
                    std::cerr << fs_Err2 << std::endl;
                    return mSPFont;
                }
            }
        }
        std::string FPCP_Root = mSPFResource_S.substr(S, C - S);
        while(mSPFResource_S[C] != ' '){
            C++;
            if(C >= mSPFResource_S.size()){
                std::cerr << fs_Err2 << std::endl;
                return mSPFont;
            }
        }
        C++;

        try{
            S = C;
            while(mSPFResource_S[C] != ' '){
                C++;
                if(C >= mSPFResource_S.size()){
                    std::cerr << fs_Err2 << std::endl;
                    return mSPFont;
                }
            }
            fxCutFont.CharCX = std::stoi(mSPFResource_S.substr(S, C - S));
            S = ++C;
            while(mSPFResource_S[C] != ' '){
                C++;
                if(C >= mSPFResource_S.size()){
                    std::cerr << fs_Err2 << std::endl;
                    return mSPFont;
                }
            }
            fxCutFont.CharCY = std::stoi(mSPFResource_S.substr(S, C - S));
            S = ++C;
            while(mSPFResource_S[C] != ' '){
                C++;
                if(C >= mSPFResource_S.size()){
                    std::cerr << fs_Err2 << std::endl;
                    return mSPFont;
                }
            }
            fxCutFont.SCD = std::stoi(mSPFResource_S.substr(S, C - S), nullptr, 0);
            C++;
            if(C >= mSPFResource_S.size()){
                std::cerr << fs_Err2 << std::endl;
                return mSPFont;
            }
            fxCutFont.CCX = mSPFResource_S[C];
        }catch(...){
            std::cerr << fs_Err4_1;
            return mSPFont;
        }

        int CCXInt = 0;
        switch(fxCutFont.CCX){
            case '1':
            case 'B':
            case 'b':
                CCXInt = 1;
                break;
            case '2':
            case 'G':
            case 'g':
                CCXInt = 2;
            case '3':
            case 'R':
            case 'r':
                CCXInt = 3;
            default:
                CCXInt = 0;
        }
        fontPack cpdFontPack = importFont_root(mSPFResource_S.substr(S, C - S), sCDimensions.first, sCDimensions.second, fxCutFont.SCD, ((fxCutFont.CharCX * fxCutFont.CharCY) + fxCutFont.SCD) - 1, CCXInt);
        mSPFont.fontPackCPD[tNameFP] = cpdFontPack;
        C++;
        if(C < mSPFResource_S.size()) C += mSPFResource_S[C] == '\r' ? 2 : 1;
    }
    mSPFont.charDS = ivDSize;
    return mSPFont;
}

// Too many errors on this function. Please if you find an error, resolve it and take a pull request for me.
SPFontPack load_SPFont(std::string ObjName){
    SPFontPack mSPFont;
    const std::string fs_Err0 = "Looks like the the Font Data Identifier Parser was deleted or cleared.\nThis is a sacred file and it helps you to charge fonts and for save other configurations\n\nIf the file is in the recycle bin (trash), restore it or else, the app will not run.\nThe root of the file is assigned as: font/.root.fnt.csv (load_SPFont / err 0.0)";
    const std::string fs_Err4 = "Deftoken name is all spaced (load_SPFont / err 4)";
    const std::string fs_Err5 = "Parser ended too early (load_SPFont / err 5)";
    const std::string fs_Err8 = "Negative value on Substring function (load_SPFont / err 8)";

    std::ifstream mFPars("font/.root.fnt.csv");
    if(!mFPars.is_open()){
        std::cerr << fs_Err0 << std::endl;
        return mSPFont;
    }

    std::string mFPars_conta((std::istreambuf_iterator<char>(mFPars)), std::istreambuf_iterator<char>());
    mFPars.close();
    if(mFPars_conta.empty()){
        std::cerr << fs_Err0 << std::endl;
        return mSPFont;
    }
    if(ObjName.empty()){
        std::cerr << "Font name or token inserted is empty (load_SPFont / err 0.1)" << std::endl;
        return mSPFont;
    }

    // pobj = parser object
    struct dataFontDisp_pobj{
        std::string name;
        float sizeDisp;
    };
    std::unordered_map<std::string, dataFontDisp_pobj> ipv_FDefaults;
    std::string defTName;
    int C = 0;
    while(mFPars_conta[C] != '\r' || mFPars_conta[C] != '\n'){
        dataFontDisp_pobj mDFD_PO;
        defTName = mFPars_conta.substr(C, 4);
        C += 4;
        if(mFPars_conta[C] != ' '){
            std::cerr << "Default's tokens must have 4 characters or the parser ended too early (load_SPFont / err 1.0)" << std::endl;
            return mSPFont;
        }

        if(defTName.size() != 4){
            std::cerr << "variable: \"defTName\" is not 4 char length (load_SPFont / err 2)" << std::endl;
            return mSPFont;
        }
        for(int CHC = 0, CHC < 4, CHC++){
            if(defTName[CHC] == ' '){
                std::cerr << "Default's tokens must have 4 characters (load_SPFont / err 1.1)" << std::endl;
                return mSPFont;
            }
        }
        C++;

        mDFD_PO.name.clear();
        int StartText = C;
        while(mFPars_conta[C] != '$'){
            C++;
            if(C >= static_cast<int>(mFPars_conta.size()){
                std::cerr << "The character '$' was never found during parsing (load_SPFont / err 3)" << std::endl;
                return mSPFont;
            }
        }
        if(C - StartText < 0){
            std::cerr << fs_Err8 << std::endl;
            return mSPFont;
        }
        mDFD_PO.name = mFPars_conta.substr(StartText, C - StartText);
        C++;
        if(mDFD_PO.name.empty()){
            std::cerr << "Deftoken name is empty (load_SPFont / err 4)" << std::endl;
            return mSPFont;
        }
        bool SCompDone = false;
        if(mDFD_PO.name[mDFD_PO.name.size() - 1] == ' '){
            for(size_t CHC = mDFD_PO.name.size(); CHC > 0; CHC--){
                if(mDFD_PO.name[CHC - 1] != ' '){
                    SCompDone = true;
                    mDFD_PO.name = mDFD_PO.name.substr(0, CHC);
                    break;
                }
            }
            if(!SCompDone) std::cerr << fs_Err4 << std::endl;
        }
        if(mDFD_PO.name[0] == ' ' && SCompDone){
            SCompDone = false;
            for(size_t CHC = 0; CHC < mDFD_PO.name.size(); CHC++){
                if(mDFD_PO.name[CHC] != ' '){
                    SCompDone = true;
                    mDFD_PO.name = mDFD_PO.name.substr(CHC, mDFD_PO.name.size() - CHC);
                    break;
                }
            }
            if(!SCompDone) std::cerr << fs_Err4 << std::endl;
        }
        if(!SCompDone) return mSPFont;

        while(mFPars_conta[C] == ' ' && C < static_cast<int>(mFPars_conta.size())) C++;
        if(C >= static_cast<int>(mFPars_conta.size())){
            std::cerr << fs_Err5 << std::endl;
            return mSPFont;
        }

        if(mFPars_conta[C] == '\r' || mFPars_conta[C] == '\n'){
            mDFD_PO.sizeDisp = 1.0f;
            ipv_FDefaults[defTName] = mDFD_PO;
            C += mFPars_conta[C] == '\r' ? 2 : 1;
            continue;
        }
        StartText = C;
        std::string STRF_conv;
        while(mFPars_conta[C] != '\r' && mFPars_conta[C] != '\n'){
            C++
            if(C >= static_cast<int>(mFPars_conta.size())){
                std::cerr << fs_Err5 << std::endl;
                return mSPFont;
            }            
        }
        if(C - StartText < 0){
            std::cerr << fs_Err8 << std::endl;
            return mSPFont;
        }
        STRF_conv = mFPars_conta.substr(StartText, C - StartText);
        try{
            mDFD_PO.sizeDisp = std::stof(STRF_conv);
        }catch(...){
            std::cerr << "miswritten or corrupted float number writted on the parser (load_SPFont / err 6)" << std::endl;
            return mSPFont;
        }
        ipv_FDefaults[defTName] = mDFD_PO;
        C += mFPars_conta[C] == '\r' ? 2 : 1;
    }
    char mChar = mFPars_conta[C];
    const std::string fs_Err7 = "Unexpected character after end (load_SPFont / err 7)";
    if(mChar == '\r' || mChar == '\n'){
        C += mChar == '\r' ? 2 : 1;
    }else{
        std::cerr << fs_Err7 << std::endl;
        return mSPFont;
    }

    while(mFPars_conta[C] == '\r' || mFPars_conta[C] == '\n'){
        C += mFPars_conta[C] == '\r' ? 2 : 1;
    }

    std::unordered_map<std::string, std::string> ipv_FRoots; // {Font Name, Font Root}
    std::pair<std::string, std::string> FRoots_pobj;
    int StartName = C;
    while(C < mFPars_conta.size() && mFPars_conta[C] != '\r' && mFPars_conta[C] != '\n'){
        StartName = C;
        while(mFPars_conta[C] != '$'){
            C++;
            if(C >= static_cast<int>(mFPars_conta.size()){
                std::cerr << "The character '$' was never found during parsing (load_SPFont / err 3)" << std::endl;
                return mSPFont;
            }
        }
        if(C - StartName < 0){
            std::cerr << fs_Err8 << std::endl;
            return mSPFont;
        }
        FRoots_pobj.first = mFPars_conta.substr(StartName, C - StartName);
        if(FRoots_pobj.first.empty()){
            std::cout << "Font Name is empty or failed to load (load_SPFont / err 9)" << std::endl;
            return mSPFont;
        }

        bool SCompDone = false;
        if(FRoots_pobj.first[FRoots_pobj.first.size() - 1] == ' '){
            for(size_t CHC = FRoots_pobj.first.size(); CHC > 0; CHC--){
                if(FRoots_pobj.first[CHC - 1] != ' '){
                    SCompDone = true;
                    FRoots_pobj.first = FRoots_pobj.first.substr(0, CHC);
                    break;
                }
            }
            if(!SCompDone) std::cerr << fs_Err4 << std::endl;
        }
        if(FRoots_pobj.first[0] == ' ' && SCompDone){
            SCompDone = false;
            for(size_t CHC = 0; CHC < FRoots_pobj.first.size(); CHC++){
                if(FRoots_pobj.first[CHC] != ' '){
                    SCompDone = true;
                    FRoots_pobj.first = FRoots_pobj.first.substr(CHC, FRoots_pobj.first.size() - CHC);
                    break;
                }
            }
            if(!SCompDone) std::cerr << fs_Err4 << std::endl;
        }
        if(!SCompDone) return mSPFont;

        C++
        while(mFPars_conta[C] == ' ' && C < mFPars_conta.size()) C++;
        if(C >= mFPars_conta.size()){
            std::cerr << fs_Err5 << std::endl;
            return mSPFont;
        }

        StartName = C;
        while(mFPars_conta[C] != '\r' || mFPars_conta[C] != '\n' || C < mFPars_conta.size()){
            C++
        }
        if(C - StartName < 0){
            std::cerr << fs_Err8 << std::endl;
            return mSPFont;
        }
        FRoots_pobj.second = mFPars_conta.substr(StartName, C - StartName);

        SCompDone = false;
        if(FRoots_pobj.second[FRoots_pobj.second.size() - 1] == ' '){
            for(size_t CHC = FRoots_pobj.second.size(); CHC > 0; CHC--){
                if(FRoots_pobj.second[CHC - 1] != ' '){
                    SCompDone = true;
                    FRoots_pobj.second = FRoots_pobj.first.substr(0, CHC);
                    break;
                }
            }
            if(!SCompDone){
                std::cerr << fs_Err4 << std::endl;
                return mSPFont;
            }
        }

        ipv_FRoots[FRoots_pobj.first] = FRoots_pobj.second;
        if(C < mFPars_conta.size()){
            mChar = mFPars_conta[C];
            if(mChar == '\r' || mChar == '\n'){
                C += mFPars_conta[C] == '\r' ? 2 : 1;
            }else{
                std::cerr << fs_Err7 << std::endl;
                return mSPFont;
            }
        }
    }

    std::string mfit_FontToken2P; // Very Necesary
    if(ObjName[0] == '#'){
        auto FDefaultFind = ipv_FDefaults.find(ObjName.substr(1, 4));
        if(FDefaultFind != ipv_FDefaults.end()){
            mfit_FontToken2P = FDefaultFint->second.name;
        }else{
            std::cerr << "Default token name doesn't exist on the parser (load_SPFont / err 10)" << std::endl;
            return mSPFont;
        }
    }else{
        mfit_FontToken2P = ObjName;
    }
    auto SFRootFind = ipv_FRoots.find(mfit_FontToken2P); 
    if(SFRootFind != ipv_FRoots.end()){
        mfit_FontToken2P = SFRootFind->second;
        mSPFont = loadResource_SPFont(mfit_FontToken2P);
        if(mSPFont.charDS == 0) std::cerr << "Smart Portable Font failed to extract, is corrupted or it was saved incorrectly (load_SPFont / err 12)" << std::endl;
        return mSPFont;
    }
    std::cerr << "Root or token doesn't exist on the parser (load_SPFont / err 11)" << std::endl;
    return mSPFont;
}

int main(){
    SDL_Window *win = nullptr;
    SDL_Renderer *ren = nullptr;

    if(!SDL_Init(SDL_INIT_VIDEO)){
        return 1;
    }

    win = SDL_CreateWindow("Gemdrive Giacavi 1.0a", 1024, 680, SDL_WINDOW_RESIZABLE);
    if(win == nullptr){
        SDL_Quit();
        return 1;
    }
    SDL_SetWindowMinimumSize(win, 380, 260);

    ren = SDL_CreateRenderer(win, nullptr);
    if(ren == nullptr){
        SDL_DestroyWindow(win);
        SDL_Quit();
        return 1;
    }
    SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_BLEND);

    bool quit = false;
    SDL_Event event;
    SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
    std::u16string mCar_Text = UTF8_to_16b_SS("#include <stdio.h>\r\n#include <stdlib.h>\r\nint main(){\r\n\tchar* stringExample = (char*)malloc(12);\r\n\tif(stringExample != NULL){\r\n\t\tstringExample = \"Hello World\";\r\n\t\tprintf(%s, stringExample);\r\n\t\tfree(stringExample);\r\n\t}\r\n\treturn 0;\r\n}\r\n");

    // Fonts
    const fontPack FNT_Premier_Classic = importFont_root("font/fontN-Wwes-12-20 Premier_Classic.lct", 12, 20, 0x20, 0xFF, 2);

    // Color Packs
    std::unordered_map<std::string, std::pair<uint32_t, uint32_t>> mDLStyle = importDataList("data/mainTheme.csv");

    std::pair<int,int> winSize;
    SDL_GetWindowSizeInPixels(win, &winSize.first, &winSize.second);

    SPFontPack mSPFP_00 = load_SPFont("Bubblegum"); // Released by testing

    while(!quit){
        while(SDL_PollEvent(&event)){
            switch(event.type){
                case SDL_EVENT_QUIT:
                    quit = true;
                    break;
                case SDL_EVENT_WINDOW_RESIZED:
                    SDL_GetWindowSizeInPixels(win, &winSize.first, &winSize.second);
                    break;
                case SDL_EVENT_MOUSE_BUTTON_DOWN:
                    if(event.button.button == SDL_BUTTON_LEFT){
                    }
                    break;
            }
        }
        SDL_RenderClear(ren);

        ArrayDText_UTF16(
            ren,
            FNT_Premier_Classic,
            mDLStyle,
            mCar_Text,
            winSize.first, winSize.second, 0, 30, 0, 0, 1.0f
        );

        createButtonFlow_tex_X(
            ren, 4,
            {0xFFFFE080, 0xFFFFD426, 0xFFB8AD10}, 3, {0xFF403C10, 0xFF302A0C}, 2,
            0.0f, 0.0f, 80.0f, 24.0f, 2.0f
        );

        SDL_RenderPresent(ren);
        SDL_Delay(33);
    }

    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}