#ifndef LCONSOLETEX_V1_DECODE
#define LCONSOLETEX_V1_DECODE

#include <stdint.h>
#include <stddef.h>

struct imageLCT_C {
    unsigned char* raw;
    size_t raw_size;
    uint32_t da;
    uint32_t de;
    int C;
};

#ifdef __cplusplus
    #include <vector>

    extern std::vector<unsigned char> importBuffer_root(const char* root);
    extern struct imageLCT;
    extern imageLCT importLCT_data(const std::vector<unsigned char>& mainLCT); // The decodifier content will not be show because it's private, untill 12-Jan-2026
    extern imageLCT_C dataCpp_C(imageLCT_C mainLCTdata_CPP);

    extern "C" {
#endif

imageLCT_C importLCT_root(const char* root);
void free_CPP(unsigned char* rawP);

#ifdef __cplusplus
    }
#endif

#endif // LCT Header
