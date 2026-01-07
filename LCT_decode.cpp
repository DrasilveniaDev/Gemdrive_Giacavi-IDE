#include <utility>
#include <vector>
#include <cstdint>
#include <cstdstring>
#include <cstdlib>
#include <cstddef>
#include <unordered_map>
#include "LCT_decode.h"

struct imageLCT {
    std::vector<unsigned char> raw;
    int da;
    int de;
    int C;
};

imageLCT importLCT_data(const std::vector<unsigned char>& mainLCT){
    imageLCT LCTdata_O;

    LCTdata_O.da = (mainLCT[6] << 4) | (mainLCT[7] >> 4);
    LCTdata_O.de = ((mainLCT[7] & 0xF) << 8) | mainLCT[8];
    LCTdata_O.da++;
    LCTdata_O.de++;
    
    // After this code, the decodifier code will not be showed untill 12-Jan-2026 because it's private.
    // The following code is a placeholder
    
    LCTdata_O.raw = {0};
    LCTdata_O.C = 0;
    return LCTdata_O;
}

imageLCT_C dataCpp_C(imageLCT mainLCTdata_CPP){
    imageLCT_C LCTdata_OC;
    const size_t raw_size_CPP = mainLCTdata_CPP.raw.size();

    LCTdata_OC.raw_size = raw_size_CPP;

    LCTdata_OC.da = (uint32_t)mainLCTdata_CPP.da;
    LCTdata_OC.de = (uint32_t)mainLCTdata_CPP.de;
    LCTdata_OC.C = mainLCTdata_CPP.C;

    if(LCTdata_OC.C == 0 || LCTdata_OC.raw_size < 1){
        LCTdata_OC.raw = NULL;
    }else{
        LCTdata_OC.raw = (unsigned char*)std::malloc(raw_size_CPP);
        if(LCTdata_OC.raw) std::memcpy(LCTdata_OC.raw, mainLCTdata_CPP.raw.data(), raw_size_CPP);
    }
    return LCTdata_OC;
}

std::vector<unsigned char> importBuffer_root(const char* root) {
    std::ifstream file(root, std::ios::binary | std::ios::ate);
    if(!file.is_open()) {
        return {};
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    std::vector<unsigned char> BufferFile(size);
    if(file.read(reinterpret_cast<char*>(BufferFile.data()), size)){
        return BufferFile;
    }
    return {};
}

extern "C" imageLCT_C importLCT_root(const char* root){
    imageLCT_C DPackError = {NULL, 0, 0, 0, 0};
    std::vector<unsigned char> MBuffer = importBuffer_root(root);
    if(MBuffer.empty()) return DPackError;

    return dataCpp_C(importLCT_data(MBuffer));
}

extern "C" void free_CPP(unsigned char* rawP){
    std::free(rawP);
}
