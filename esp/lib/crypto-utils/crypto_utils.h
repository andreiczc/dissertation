#ifndef _CRYPTO_UTILS_H
#define _CRYPTO_UTILS_H

#include <memory>

namespace crypto
{
std::unique_ptr<uint8_t[]> encodeBase64(uint8_t *input);

std::unique_ptr<uint8_t[]> decodeBase64(uint8_t *input);

std::unique_ptr<uint8_t[]> encryptAes(uint8_t *input);
} // end namespace crypto

#endif // _CRYPTO_UTILS_H