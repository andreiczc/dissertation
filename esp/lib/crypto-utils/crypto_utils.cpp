#include "crypto_utils.h"

#include "aes/esp_aes.h"
#include "esp_log.h"
#include "mbedtls/base64.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/entropy.h"
#include <cstring>
#include <functional>

#define KEY_SIZE   32
#define BLOCK_SIZE 16

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

  if (padBytes < BLOCK_SIZE)
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
  ESP_LOG_BUFFER_HEX(TAG, key, KEY_SIZE);
  ESP_LOG_BUFFER_HEX(TAG, iv, BLOCK_SIZE);

  outputSize = inputLength % BLOCK_SIZE == 0
                   ? inputLength
                   : inputLength + (BLOCK_SIZE - inputLength % BLOCK_SIZE);

  std::unique_ptr<uint8_t[]> output(new uint8_t[outputSize]);

  esp_aes_context context;
  esp_aes_init(&context);
  esp_aes_setkey(&context, key, KEY_SIZE * 8);

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
  ESP_LOG_BUFFER_HEX(TAG, key, KEY_SIZE);
  ESP_LOG_BUFFER_HEX(TAG, iv, BLOCK_SIZE);

  std::unique_ptr<uint8_t[]> output(new uint8_t[inputLength]);

  esp_aes_context context;
  esp_aes_init(&context);
  esp_aes_setkey(&context, key, KEY_SIZE * 8);

  const auto returnValue = esp_aes_crypt_cbc(
      &context, ESP_AES_DECRYPT, inputLength, iv, input, output.get());

  ESP_LOGI(TAG, "Decrypt return value: %d", returnValue);
  ESP_LOG_BUFFER_HEX(TAG, output.get(), inputLength);

  auto depadded = depadPkcs5(output.get(), inputLength);

  ESP_LOG_BUFFER_HEX(TAG, depadded.get(), inputLength);

  esp_aes_free(&context);

  return std::move(depadded);
}

mbedtls_ecdh_context generateEcdhParams()
{
  ESP_LOGI(TAG, "Generating ECDH params");

  mbedtls_ecdh_context     context;
  mbedtls_ctr_drbg_context ctrDrbg;
  mbedtls_entropy_context  entropy;
  const auto              *custom = "l33t";

  mbedtls_ecdh_init(&context);
  mbedtls_ctr_drbg_init(&ctrDrbg);
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

  uint8_t buffer[KEY_SIZE];
  returnValue = mbedtls_mpi_write_binary(&context.Q.X, buffer, KEY_SIZE);
  ESP_LOGI(TAG, "mbedtls_mpi_write_binary return code: %d", returnValue);
  ESP_LOG_BUFFER_HEX(TAG, buffer, KEY_SIZE);

  mbedtls_entropy_free(&entropy);
  mbedtls_ctr_drbg_free(&ctrDrbg);

  return context;
}

std::unique_ptr<uint8_t[]> generateSharedSecret(mbedtls_ecdh_context &context,
                                                uint8_t *peerPublicParam)
{
  auto returnCode = mbedtls_mpi_lset(&context.Qp.Z, 1);
  ESP_LOGI(TAG, "mbedtls_mpi_lset return code: %d", returnCode);

  returnCode =
      mbedtls_mpi_read_binary(&context.Qp.X, peerPublicParam, KEY_SIZE);
  ESP_LOGI(TAG, "mbedtls_mpi_read_binary return code: %d", returnCode);

  mbedtls_ctr_drbg_context ctrDrbg;
  mbedtls_entropy_context  entropy;
  const auto              *custom = "l33t";

  mbedtls_entropy_init(&entropy);
  returnCode = mbedtls_ctr_drbg_seed(&ctrDrbg, mbedtls_entropy_func, &entropy,
                                     (uint8_t *)custom, strlen(custom));
  ESP_LOGI(TAG, "mbedtls_ctr_drbg_seed return code: %d", returnCode);

  returnCode = mbedtls_ecdh_compute_shared(&context.grp, &context.z,
                                           &context.Qp, &context.d,
                                           mbedtls_ctr_drbg_random, &ctrDrbg);
  ESP_LOGI(TAG, "mbedtls_ecdh_compute_shared return code: %d", returnCode);

  std::unique_ptr<uint8_t[]> buffer(new uint8_t[KEY_SIZE]);
  returnCode = mbedtls_mpi_write_binary(&context.z, buffer.get(), KEY_SIZE);
  ESP_LOGI(TAG, "mbedtls_mpi_write_binary return code: %d", returnCode);
  ESP_LOG_BUFFER_HEX(TAG, buffer.get(), KEY_SIZE);

  mbedtls_entropy_free(&entropy);
  mbedtls_ctr_drbg_free(&ctrDrbg);

  return std::move(buffer);
}
} // end namespace crypto