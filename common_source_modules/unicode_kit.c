#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include "unicode_kit.h"

U16char_ps UTF8_to_16b(const char* mString){
    int UD = 0;
    size_t m16S_size = 0;
    for(int C = 0; mString[C] != '\0'; C++){
        unsigned char CUV = (unsigned char)mString[C];
        if(CUV >= 0xC0 && CUV < 0xE0){
            UD = 1;
        }else if(CUV >= 0xE0 && CUV < 0xF0){
            UD = 2;
        }

        if(UD <= 0){
            m16S_size++;
            if(UD < 0) UD = 0;
        }else{
           UD--;
        }
    }
    m16S_size++;
    if(m16S_size == 0){
        fprintf(stderr, "String is empty\n");
        return (U16char_ps){NULL, 0};
    }

    uint16_t *m16S = (uint16_t*)malloc(sizeof(uint16_t) * m16S_size);
    if(m16S == NULL){
        fprintf(stderr, "16bit String didn't allocate\n");
        return (U16char_ps){NULL, 0};
    }

    int C = 0;
    int CW = 0;
    uint16_t c16b_hold = 0;
    while(CW < (int)m16S_size){
        uint16_t c16b = 0;
        unsigned char CUV = (unsigned char)mString[C];

        if(mString[C] < 0 || CUV > 0x7F){
            if(CUV > 0xBF && CUV < 0xE0){
                c16b = (uint16_t)(CUV & 0x1F) << 6;
                c16b |= (uint16_t)(unsigned char)mString[++C] & 0x3F;
                m16S[CW++] = c16b;
                C++;
                continue;
            }
            if(CUV > 0xDF && CUV < 0xF0){
                c16b = (uint16_t)(CUV & 0x0F) << 12;
                c16b_hold = (uint16_t)(unsigned char)mString[++C] & 0x3F;
                c16b |= c16b_hold << 6;
                c16b |= (uint16_t)(unsigned char)mString[++C] & 0x3F;
                m16S[CW++] = c16b;
                C++;
                continue;
            }
            if(CUV > 0xEF || (CUV > 0x7F && CUV < 0xC0)){
                fprintf(stderr, "UTF-8 String is corrupt or the function has an erroneous operation\n");
                return (U16char_ps){NULL, 0};
            }
        }

        c16b = CUV;
        m16S[CW++] = c16b;
        C++;
    }

    U16char_ps p16S = {m16S, m16S_size};
    return p16S;
}

const charD_16b_8b win1252_d[28] = {
    {0x20AC, 0x80},
    {0x201A, 0x82},
    {0x0192, 0x83},
    {0x201E, 0x84},
    {0x2026, 0x85},
    {0x2020, 0x86},
    {0x2021, 0x87},

    {0x02C6, 0x88},
    {0x2030, 0x89},
    {0x0160, 0x8A},
    {0x2039, 0x8B},
    {0x0152, 0x8C},
    {0x017D, 0x8E},

    {0x2018, 0x91},
    {0x2019, 0x92},
    {0x201C, 0x93},
    {0x201D, 0x94},
    {0x2022, 0x95},
    {0x2013, 0x96},
    {0x2014, 0x97},

    {0x02DC, 0x98},
    {0x2122, 0x99},
    {0x0161, 0x9A},
    {0x203A, 0x9B},
    {0x0153, 0x9C},
    {0x017E, 0x9E},
    {0x0178, 0x9F},
    {0, 0}
};

char *S16b_to_win1252(const U16char_ps mp16String){
    if(mp16String.string == NULL || mp16String.size == 0){
        fprintf(stderr, "16b String inserted is Empty or Corrupt\n");
        return NULL;
    }
    char *mString = (char*)malloc(mp16String.size);

    if(mString == NULL){
        fprintf(stderr, "8b String not allocated\n");
        return NULL;
    }

    for(size_t CHC = 0; CHC < mp16String.size; CHC++){
        uint16_t A16char = mp16String.string[CHC];
        unsigned char A8char = '?';

        int TB0 = 0;
        int CHC_D = 0;
        while(win1252_d[CHC_D].char16b != 0){
            if(A16char == win1252_d[CHC_D].char16b){
                A8char = win1252_d[CHC_D].char8b;
                TB0 = 1;
                break;
            }
            CHC_D++;
        }
        if(TB0 == 0){
            if(A16char < 128 || (A16char >= 0xA0 && A16char < 0x100)){
                A8char = (unsigned char)(A16char & 0xFF);
            }
        }
        mString[CHC] = (char)A8char;
    }
    return mString;
}
void S16b_free(uint16_t *mU16_string){
    if(mU16_string != NULL){
        free(mU16_string);
    }
}
void S8b_free(char *mString){
    if(mString != NULL){
        free(mString);
    }
}
