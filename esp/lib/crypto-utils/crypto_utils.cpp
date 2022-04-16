#include "crypto_utils.h"

#include "aes/esp_aes.h"
#include "esp_log.h"
#include "mbedtls/base64.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/ecdh.h"
#include "mbedtls/entropy.h"
#include <cstring>

namespace crypto
{
static auto *TAG = "CRYPTO";

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

static std::unique_ptr<uint8_t[]> depadPkcs5(uint8_t *input, size_t inputLength)
{
  std::unique_ptr<uint8_t[]> result(new uint8_t[inputLength]);

  const auto padBytes = input[inputLength - 1];

  memcpy(result.get(), input, inputLength);

  if (padBytes < 16)
  {
    for (auto i = inputLength - 1; i >= inputLength - padBytes; --i)
    {
      if (result.get()[i] != padBytes)
      {
        break;
      }

      result.get()[i] = 0;
    }
  }

  return std::move(result);
}

std::unique_ptr<uint8_t[]> encryptAes(uint8_t *input, uint8_t *key, uint8_t *iv,
                                      uint16_t &outputSize)
{
  // TODO verify key length

  const auto inputLength = strlen((char *)input);
  ESP_LOGI(TAG, "Plaintext size: %d", inputLength);
  ESP_LOG_BUFFER_HEX(TAG, key, 32);
  ESP_LOG_BUFFER_HEX(TAG, iv, 16);

  outputSize = inputLength % 16 == 0 ? inputLength
                                     : inputLength + (16 - inputLength % 16);

  std::unique_ptr<uint8_t[]> output(new uint8_t[outputSize]);

  esp_aes_context context;
  esp_aes_init(&context);
  esp_aes_setkey(&context, key, 256);

  int returnValue = 0;

  if (outputSize != inputLength)
  {
    ESP_LOGI(TAG, "Input will be padded PKCS5");
    auto paddedInput = padPkcs5(input, inputLength, outputSize);
    ESP_LOG_BUFFER_HEX(TAG, paddedInput.get(), outputSize);

    returnValue = esp_aes_crypt_cbc(&context, ESP_AES_ENCRYPT, outputSize, iv,
                                    paddedInput.get(), output.get());
  }
  else
  {
    returnValue = esp_aes_crypt_cbc(&context, ESP_AES_ENCRYPT, outputSize, iv,
                                    input, output.get());
  }

  esp_aes_free(&context);

  ESP_LOGI(TAG, "Encrypt return value: %d", returnValue);
  ESP_LOG_BUFFER_HEX(TAG, output.get(), outputSize);

  return std::move(output);
}

std::unique_ptr<uint8_t[]> decryptAes(uint8_t *input, uint16_t inputLength,
                                      uint8_t *key, uint8_t *iv)
{
  ESP_LOGI(TAG, "Size of ciphertext: %d", inputLength);
  ESP_LOG_BUFFER_HEX(TAG, key, 32);
  ESP_LOG_BUFFER_HEX(TAG, iv, 16);

  std::unique_ptr<uint8_t[]> output(new uint8_t[inputLength]);

  esp_aes_context context;
  esp_aes_init(&context);
  esp_aes_setkey(&context, key, 256);

  const auto returnValue = esp_aes_crypt_cbc(
      &context, ESP_AES_DECRYPT, inputLength, iv, input, output.get());

  ESP_LOGI(TAG, "Decrypt return value: %d", returnValue);
  ESP_LOG_BUFFER_HEX(TAG, output.get(), inputLength);

  auto depadded = depadPkcs5(output.get(), inputLength);

  ESP_LOG_BUFFER_HEX(TAG, depadded.get(), inputLength);

  esp_aes_free(&context);

  return std::move(depadded);
}

std::unique_ptr<uint8_t[]> generateEcdhParams()
{
  std::unique_ptr<uint8_t[]> result(new uint8_t[32]);
  mbedtls_ecdh_context       context;
  mbedtls_ctr_drbg_context   ctrDrbg;
  mbedtls_entropy_context    entropy;
  const auto                *custom = "l33t";

  mbedtls_ecdh_init(&context);
  mbedtls_entropy_init(&entropy);

  auto returnValue =
      mbedtls_ecp_group_load(&context.grp, MBEDTLS_ECP_DP_BP256R1);
  ESP_LOGI(TAG, "mbedtls_ecp_group_load return code: %d", returnValue);

  returnValue = mbedtls_ctr_drbg_seed(&ctrDrbg, mbedtls_entropy_func, &entropy,
                                      (uint8_t *)custom, strlen(custom));
  ESP_LOGI(TAG, "mbedtls_ctr_drbg_seed return code: %d", returnValue);

  returnValue = mbedtls_ecdh_gen_public(&context.grp, &context.d, &context.Q,
                                        mbedtls_ctr_drbg_random, &ctrDrbg);
  ESP_LOGI(TAG, "mbedtls_ecdh_gen_public return code: %d", returnValue);

  returnValue = mbedtls_mpi_write_binary(&context.Q.X, result.get(), 32);
  ESP_LOGI(TAG, "mbedtls_mpi_write_binary return code: %d", returnValue);

  return std::move(result);
}
} // end namespace crypto