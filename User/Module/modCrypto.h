

#ifndef __MODULE_CRYPTO__
#define __MODULE_CRYPTO__

#include "main.h"

extern void aes_Init(void);
extern void encrypt_AES_ECB(uint8_t *input, uint8_t *output, uint8_t *key, uint16_t keybits);
extern void decrypt_AES_ECB(uint8_t *input, uint8_t *output, uint8_t *key, uint16_t keybits);
extern void encrypt_AES_CBC(uint8_t *input, uint8_t *output, uint8_t *key, uint16_t keybits);
extern void decrypt_AES_CBC(uint8_t *input, uint8_t *output, uint8_t *key, uint16_t keybits);

#endif
