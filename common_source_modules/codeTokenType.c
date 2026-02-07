#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include "codeTokenType.h"

const token_ident_dict cd_tokenTypeD[11] = {
    {0, "SText"},
    {1, "SDeclaration"},
    {2, "SNote"},
    {3, "SPunctuation"},
    {4, "SOperation"},
    {5, "SComparison"},
    {6, "SNumber"},
    {7, "SPunctuationC"},
    {8, "SChar"},
    {9, "SString"},
    {-1, "!END_READ"}
};

const char* get_tokenNameF_int(int mTokenInt){
    for(int W = 0; W < 11; W++){
        if(mTokenInt == cd_tokenTypeD[W].co_token_int) return cd_tokenTypeD[W].co_token_name;
    }
    return "!NO_ONE_FIND";
}

int* categorizeCode_token(const uint16_t* mTCode){
    if(mTCode == NULL) return NULL;
    if(sizeof(int) != 4){
        fprintf(stderr, "int cast is not 4B size\n");
        return NULL;
    }
    
    size_t mTCode_size = 0;
    int CHC = 0;
    while(mTCode[CHC] != 0){
        mTCode_size = ++CHC;
    }
    CHC++;
    
    int *mTISC = (int*)malloc(sizeof(int) * CHC);
    if(mTISC == NULL) return NULL;
    
    CHC = 0;
    
    struct charCooldown{
        char cool_char;
        int cool_delay;
    };
    struct charCooldown itsc_CDSelect = {0, 0};
    int cool_luck = 0;
    int token_markTextCategory = 0;
    do{
        uint16_t mCharRS = mTCode[CHC];
        if(itsc_CDSelect.cool_char == mCharRS || itsc_CDSelect.cool_delay == 0 || cool_luck == 1){
            itsc_CDSelect.cool_delay -= itsc_CDSelect.cool_delay != 0 ? 1 : 0;
            if(itsc_CDSelect.cool_delay < 0) itsc_CDSelect.cool_delay = 0;
            
            if(itsc_CDSelect.cool_delay == 0){
                cool_luck = 0;
                
                // Zone for the IDE.
                // This is for paint the IDE with the correspondent color for each category.
                // Take in care that this still needs to be edit because in any moment you will notice
                // that it stills without end.
                switch(mCharRS){
                    case 0x0023:
                        token_markTextCategory = 1;
                        itsc_CDSelect.cool_char = '\n';
                        itsc_CDSelect.cool_delay = 1;
                        break;
                    case 0x0028:
                    case 0x0029:
                    case 0x005B:
                    case 0x005D:
                    case 0x007B:
                    case 0x007D:
                        token_markTextCategory = 3;
                        itsc_CDSelect.cool_delay = 0;
                        break;
                    case 0x002C:
                    case 0x002E:
                    case 0x003A:
                    case 0x003B:
                        token_markTextCategory = 7;
                        itsc_CDSelect.cool_delay = 0;
                        break;
                    case 0x0000:
                        token_markTextCategory = -1;
                        break;
                    default:
                        token_markTextCategory = 0;
                        itsc_CDSelect.cool_delay = 0;
                        break;
                }
            }else{
                cool_luck = 1;
            }
        }

        mTISC[CHC] = token_markTextCategory;
        if(token_markTextCategory == -1 || mCharRS == 0) break;
    }while(mTCode[CHC++] != 0);
    
    return mTISC;
}
