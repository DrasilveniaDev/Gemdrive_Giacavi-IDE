#ifndef BSOFT_LCONSOLETEX_V1_DECODE_C
#define BSOFT_LCONSOLETEX_V1_DECODE_C

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

struct imageLCT_C {
    unsigned char* raw;
    size_t raw_size;
    uint32_t width;
    uint32_t height;
    int C;
};

struct imageLCT_C importLCT_data(const unsigned char* mainLCT, const size_t mainLCT_size){
    struct imageLCT_C LCT_mPack;
    LCT_mPack.raw = NULL;
    LCT_mPack.raw_size = 0;
    LCT_mPack.C = 0; // Defines error. The final will be set as 4

    if(mainLCT == NULL) {
        fprintf(stderr, "LCT decode C: no pointer found (err 0.0)\n");
        return LCT_mPack;
    }else if(mainLCT_size == 0){
        fprintf(stderr, "LCT decode C: mainLCT_size is 0 (err 0.1)\n");
        return LCT_mPack;
    }else if(mainLCT_size < 12){
        fprintf(stderr, "LCT decode C: too small. (err 1)\n");
        return LCT_mPack;
    }

    LCT_mPack.width = (mainLCT[6] << 4) | (mainLCT[7] >> 4);
    LCT_mPack.height = ((mainLCT[7] & 0xF) << 8) | mainLCT[8];
    LCT_mPack.width++;
    LCT_mPack.height++;

    LCT_mPack.raw_size = LCT_mPack.width * 4 * LCT_mPack.height;
    LCT_mPack.raw = (unsigned char*)malloc(LCT_mPack.raw_size);
    if(!LCT_mPack.raw){
        fprintf(stderr, "LCT decode C: *raw no allocated. (err 2)\n");
        return LCT_mPack;
    }

    // The raw decoder will not be showed because the original header is private
    // It has a rule that prohibits use, copy or modify the decoder without authorization
    // So the raw decoder is Censored by that reason. Sorry

    free(mdr_pal);
    for(size_t PixC = 0; PixC < raw32_size; PixC++){
        if(mdr_raw32[PixC] <= 0xFFFFFF) mdr_raw32[PixC] |= 0xFF000000;
        for(int B = 0; B < 4; B++){
            LCT_mPack.raw[PixC * 4 + B] = (unsigned char)((mdr_raw32[PixC] >> (B * 8)) & 0xFF);
        } 
    }
    free(mdr_raw32);

    LCT_mPack.C = 4;
    return LCT_mPack;
}

struct imageLCT_C importLCT_root(const char* root){
    struct imageLCT_C mPack = (struct imageLCT_C){.raw = NULL, .raw_size = 0, .width = 0, .height = 0, .C = 0};

    FILE *file = NULL;
    long file_size = 0;
    unsigned char *mBuffer = NULL;

    file = fopen(root, "rb");
    if(file == NULL) return mPack;

    if(fseek(file, 0, SEEK_END) != 0){
        fclose(file);
        return mPack;
    }

    file_size = ftell(file);
    if(file_size == -1L){
        fclose(file);
        return mPack;
    }

    if(fseek(file, 0, SEEK_SET) != 0){
        fclose(file);
        return mPack;
    }

    mBuffer = (unsigned char*)malloc(file_size);
    if(mBuffer == NULL){
        fclose(file);
        return mPack;
    }

    size_t mBuffer_size = fread(mBuffer, 1, file_size, file);

    if(mBuffer_size != file_size){
        free(mBuffer);
        fclose(file);
        return mPack;
    }
    fclose(file);

    mPack = importLCT_data(mBuffer, mBuffer_size);
    free(mBuffer);
    
    return mPack; 
}
