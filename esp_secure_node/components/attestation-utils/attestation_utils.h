#ifndef _ATTESTATION_UTILS_H
#define _ATTESTATION_UTILS_H

#include "crypto_utils.h"
#include <Arduino.h>

namespace attestation
{
bool checkExistingKey();

std::unique_ptr<uint8_t[]> extractExistingKey();

std::unique_ptr<uint8_t[]> performClientHello();

String performKeyExchange(mbedtls_ecdh_context &ecdhParams);

std::unique_ptr<uint8_t[]>
performClientFinish(const char *publicParams, const char *signature,
                    const char *test, const uint8_t *serverPoint,
                    mbedtls_ecdh_context &ecdhParams, int &instanceId);
} // namespace attestation

#endif // _ATTESTATION_UTILS_H