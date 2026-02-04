#ifndef GEMDRIVE_UNICODE_KIT
#define GEMDRIVE_UNICODE_KIT

#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct U16char_ps{
    uint16_t *string;
    size_t size;
} U16char_ps;
U16char_ps UTF8_to_16b(const char* mString);

typedef struct charD_16b_8b{
    uint16_t char16b;
    unsigned char char8b;
} charD_16b_8b;
extern const charD_16b_8b win1252_d[28];

char *S16b_to_win1252(const U16char_ps mp16String);

void S16b_free(uint16_t *mU16_string);
void S8b_free(char *mString);

#ifdef __cplusplus
}
#endif

#endif