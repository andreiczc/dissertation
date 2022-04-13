#include "crypto_utils.h"

#include "mbedtls/aes.h"
#include "mbedtls/base64.h"
#include <cstring>

namespace crypto
{
std::unique_ptr<uint8_t[]> encodeBase64(uint8_t *input)
{
  const auto inputLength = strlen((char *)input);

  std::unique_ptr<uint8_t[]> returnValue(new uint8_t[3 * inputLength]);

  size_t actualLength = 0;
  mbedtls_base64_encode(returnValue.get(), 3 * inputLength, &actualLength,
                        input, inputLength);

  returnValue.get()[actualLength] = 0;

  return std::move(returnValue);
}

std::unique_ptr<uint8_t[]> decodeBase64(uint8_t *input)
{
  const auto inputLength = strlen((char *)input);

  std::unique_ptr<uint8_t[]> returnValue(new uint8_t[inputLength]);

  size_t actualLength = 0;
  mbedtls_base64_decode(returnValue.get(), 3 * inputLength, &actualLength,
                        input, inputLength);

  returnValue.get()[actualLength] = 0;

  return std::move(returnValue);
}

static std::unique_ptr<uint8_t[]> padPkcs5(uint8_t *input, size_t inputLength,
                                           size_t outputLength)
{
  auto padBytes = outputLength - inputLength;

  std::unique_ptr<uint8_t[]> output(new uint8_t[outputLength]);
  memcpy(output.get(), input, inputLength);
  for (auto i = inputLength; i < outputLength; ++i)
  {
    output.get()[i] = padBytes;
  }

  return std::move(output);
}

std::unique_ptr<uint8_t[]> encryptAes(uint8_t *input, uint8_t *key, uint8_t *iv)
{
  // TODO verify key length

  const auto inputLength  = strlen((char *)input);
  const auto outputLength = inputLength % 16 == 0
                                ? inputLength
                                : inputLength + (16 - inputLength % 16);

  std::unique_ptr<uint8_t[]> output(new uint8_t[outputLength]);

  mbedtls_aes_context context;
  mbedtls_aes_setkey_enc(&context, key, 256);

  if (outputLength != inputLength)
  {
    mbedtls_aes_crypt_cbc(&context, MBEDTLS_AES_ENCRYPT, outputLength, iv,
                          padPkcs5(input, inputLength, outputLength).get(),
                          output.get());
  }
  else
  {
    mbedtls_aes_crypt_cbc(&context, MBEDTLS_AES_ENCRYPT, outputLength, iv,
                          input, output.get());
  }

  return std::move(output);
}
} // end namespace crypto