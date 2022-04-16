#ifndef _CRYPTO_UTILS_H
#define _CRYPTO_UTILS_H

#include <memory>

namespace crypto
{
std::unique_ptr<uint8_t[]> encodeBase64(uint8_t *input);

std::unique_ptr<uint8_t[]> decodeBase64(uint8_t *input);

std::unique_ptr<uint8_t[]> encryptAes(uint8_t *input, uint8_t *key, uint8_t *iv,
                                      uint16_t &outputSize);

std::unique_ptr<uint8_t[]> decryptAes(uint8_t *input, uint16_t inputLength,
                                      uint8_t *key, uint8_t *iv);

std::unique_ptr<uint8_t[]> generateEcdhParams();
} // end namespace crypto

#endif // _CRYPTO_UTILS_H