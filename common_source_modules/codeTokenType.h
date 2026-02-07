#ifndef CODE_TOKENTYPE_IDENT
#define CODE_TOKENTYPE_IDENT

#ifdef __cplusplus
extern "C"{
#endif

#include <stdint.h>

typedef struct token_ident_dict {
    int co_token_int;
    const char* co_token_name;
}token_ident_dict;

extern const token_ident_dict cd_tokenTypeD[11];
const char* get_tokenNameF_int(int mTokenInt);
int* categorizeCode_token(const uint16_t* mTCode);

#ifdef __cplusplus
}
#endif

#endif