/**
 * @file modCrypto.h
 * @author Kuais
 * @brief   加解密模块
 * @version 0.1
 * @date 2022-09-01
 *
 * @copyright Copyright (c) 2022
 *
 */
#include "modCrypto.h"
#include "mbedtls/aes.h"

static mbedtls_aes_context aes;
void aes_Init(void)
{
    // mbedtls_aes_init(&aes);
}
void encrypt_AES_ECB(uint8_t *input, uint8_t *output, uint8_t *key, uint16_t keybits)
{
    mbedtls_aes_setkey_enc(&aes, key, keybits); //  set encrypt key
    mbedtls_aes_crypt_ecb(&aes, MBEDTLS_AES_ENCRYPT, input, output);
//    mbedtls_aes_free(&aes);
}
void decrypt_AES_ECB(uint8_t *input, uint8_t *output, uint8_t *key, uint16_t keybits)
{
    mbedtls_aes_setkey_dec(&aes, key, keybits); //  set decrypt key
    mbedtls_aes_crypt_ecb(&aes, MBEDTLS_AES_DECRYPT, input, output);
//    mbedtls_aes_free(&aes);
}
