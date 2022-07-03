#include "crypto_utils.h"

#include "aes/esp_aes.h"
#include "esp_log.h"
#include "mbedtls/aes.h"
#include "mbedtls/base64.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/ecdsa.h"
#include "mbedtls/entropy.h"
#include "mbedtls/sha512.h"
#include "mbedtls/x509.h"
#include "spiffs_utils.h"
#include <cstring>
#include <functional>

#define KEY_SIZE   32
#define BLOCK_SIZE 16
#define SHA_384    1

namespace crypto
{
static auto *TAG         = "CRYPTO";
static auto *CA_CRT_PATH = "/ca.crt";

static int verificationFunction(void *data, mbedtls_x509_crt *crt, int depth,
                                uint32_t *flags)
{
  char buf[1024];
  ((void)data);

  ESP_LOGI(TAG, "\nVerify requested for (Depth %d):\n", depth);
  mbedtls_x509_crt_info(buf, sizeof(buf) - 1, "", crt);
  ESP_LOGI(TAG, "%s", buf);

  if ((*flags) == 0)
    ESP_LOGI(TAG, "  This certificate has no flags\n");
  else
  {
    mbedtls_x509_crt_verify_info(buf, sizeof(buf), "  ! ", *flags);
    ESP_LOGI(TAG, "%s\n", buf);
  }

  return (0);
}

bool verifyCertificate(uint8_t *otherCertificate, size_t otherCertificateLength)
{
  ESP_LOGI(TAG, "Starting certificate verification...");
  const auto rootCaContentBase64 =
      SpiffsUtils::getInstance()->readText(CA_CRT_PATH);
  size_t     rootCaContentLength = 0;
  const auto rootCaContent =
      decodeBase64((uint8_t *)rootCaContentBase64.c_str(), rootCaContentLength);

  mbedtls_x509_crt rootCa;
  mbedtls_x509_crt_init(&rootCa);
  auto returnCode =
      mbedtls_x509_crt_parse(&rootCa, rootCaContent.get(), rootCaContentLength);
  ESP_LOGI(TAG, "mbedtls_x509_crt_parse return code: %d", returnCode);

  char buffer[256];
  mbedtls_x509_crt_info(buffer, sizeof(buffer) - 1, "", &rootCa);
  ESP_LOGI(TAG, "Root CA Info: %s", buffer);

  mbedtls_x509_crl crl;
  mbedtls_x509_crl_init(&crl);
  memset(&crl, 0, sizeof(mbedtls_x509_crl));

  mbedtls_x509_crt receivedCrt;
  mbedtls_x509_crt_init(&receivedCrt);
  returnCode = mbedtls_x509_crt_parse(&receivedCrt, otherCertificate,
                                      otherCertificateLength);
  ESP_LOGI(TAG, "mbedtls_x509_crt_parse return code: %d", returnCode);
  memset(&buffer, 0, sizeof(buffer));
  mbedtls_x509_crt_info(buffer, sizeof(buffer) - 1, "", &receivedCrt);
  ESP_LOGI(TAG, "Other party Info: %s", buffer);

  uint32_t flags;
  returnCode = mbedtls_x509_crt_verify(&receivedCrt, &rootCa, &crl, nullptr,
                                       &flags, verificationFunction, nullptr);

  ESP_LOGI(TAG, "Received certificate %s valid", returnCode ? "is not" : "is");

  mbedtls_x509_crt_free(&rootCa);
  mbedtls_x509_crt_free(&receivedCrt);
  mbedtls_x509_crl_free(&crl);

  return returnCode == 0;
}

std::unique_ptr<uint8_t[]> generateRandomSequence(size_t length)
{
  std::unique_ptr<uint8_t[]> result(new uint8_t[length]);

  for (auto i = 0; i < length; ++i)
  {
    result.get()[i] = (uint8_t)random(255);
  }

  return std::move(result);
}

String encodeBase64(uint8_t *input, size_t inputLength, size_t &outputLength)
{
  std::unique_ptr<uint8_t[]> returnValue(new uint8_t[3 * inputLength]);

  mbedtls_base64_encode(returnValue.get(), 3 * inputLength, &outputLength,
                        input, inputLength);

  returnValue.get()[outputLength] = 0;

  return String((char *)returnValue.get());
}

std::unique_ptr<uint8_t[]> decodeBase64(uint8_t *input, size_t &outputLength)
{
  const auto inputLength = strlen((char *)input);

  std::unique_ptr<uint8_t[]> returnValue(new uint8_t[inputLength]);

  mbedtls_base64_decode(returnValue.get(), 3 * inputLength, &outputLength,
                        input, inputLength);

  returnValue.get()[outputLength] = 0;

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

std::unique_ptr<uint8_t[]> encryptAes(uint8_t *input, size_t inputLength,
                                      uint8_t *key, uint8_t *iv,
                                      uint16_t &outputSize)
{
  ESP_LOGI(TAG, "Plaintext size: %d", inputLength);

  outputSize = inputLength;
  std::unique_ptr<uint8_t[]> output(new uint8_t[outputSize]);

  mbedtls_aes_context context;
  mbedtls_aes_init(&context);
  mbedtls_aes_setkey_enc(&context, key, KEY_SIZE * 8);

  auto returnValue = mbedtls_aes_crypt_cbc(
      &context, MBEDTLS_AES_ENCRYPT, inputLength, iv, input, output.get());

  mbedtls_aes_free(&context);

  ESP_LOGI(TAG, "Encrypt return value: %d", returnValue);
  ESP_LOG_BUFFER_HEX(TAG, output.get(), outputSize);

  size_t     base64Length = 0;
  const auto cipherTextBase64 =
      crypto::encodeBase64(output.get(), outputSize, base64Length);
  ESP_LOGI(TAG, "Ciphertext: %s", cipherTextBase64.c_str());

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

  mbedtls_entropy_free(&entropy);
  mbedtls_ctr_drbg_free(&ctrDrbg);

  return context;
}

std::unique_ptr<uint8_t[]>
generateSharedSecret(mbedtls_ecdh_context    &context,
                     const mbedtls_ecp_point &peerPublicParam)
{
  ESP_LOGI(TAG, "Computing shared secret");

  mbedtls_ctr_drbg_context ctrDrbg;
  mbedtls_entropy_context  entropy;
  const auto              *custom = "l33t";

  mbedtls_ctr_drbg_init(&ctrDrbg);
  mbedtls_entropy_init(&entropy);
  auto returnCode =
      mbedtls_ctr_drbg_seed(&ctrDrbg, mbedtls_entropy_func, &entropy,
                            (uint8_t *)custom, strlen(custom));
  ESP_LOGI(TAG, "mbedtls_ctr_drbg_seed return code: %d", returnCode);

  context.Qp = peerPublicParam;

  returnCode = mbedtls_ecp_check_privkey(&context.grp, &context.d);
  ESP_LOGI(TAG, "mbedtls_ecp_check_privkey return code: %d", returnCode);

  returnCode = mbedtls_ecp_check_pubkey(&context.grp, &context.Q);
  ESP_LOGI(TAG, "mbedtls_ecp_check_pubkey mine return code: %d", returnCode);

  returnCode = mbedtls_ecp_check_pubkey(&context.grp, &context.Qp);
  ESP_LOGI(TAG, "mbedtls_ecp_check_pubkey 3rd party return code: %d",
           returnCode);

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

std::unique_ptr<uint8_t[]> computeSha384(uint8_t *message, size_t messageLength)
{
  std::unique_ptr<uint8_t[]> result(new uint8_t[64]);

  ESP_LOGI(TAG, "Computing SHA384");
  ESP_LOG_BUFFER_HEX(TAG, message, messageLength);

  auto returnCode =
      mbedtls_sha512_ret(message, messageLength, result.get(), SHA_384);
  ESP_LOGI(TAG, "mbedtls_sha512_ret return code: %d", returnCode);

  return std::move(result);
}

std::unique_ptr<uint8_t[]> signEcdsa(uint8_t *message, size_t messageLength,
                                     size_t        &signatureLength,
                                     const uint8_t *privateKey,
                                     size_t         privateKeyLength)
{
  ESP_LOGI(TAG, "Signing ECDSA");

  mbedtls_ecdsa_context    context;
  mbedtls_entropy_context  entropy;
  mbedtls_ctr_drbg_context ctrDrbg;
  mbedtls_pk_context       pkContext;
  const auto              *custom = "sec";

  mbedtls_ecdsa_init(&context);
  mbedtls_entropy_init(&entropy);
  mbedtls_ctr_drbg_init(&ctrDrbg);
  mbedtls_pk_init(&pkContext);

  auto returnCode = mbedtls_pk_parse_key(&pkContext, privateKey,
                                         privateKeyLength, nullptr, 0);
  ESP_LOGI(TAG, "mbedtls_pk_parse_key return code: %d", returnCode);
  auto *keyPair = mbedtls_pk_ec(pkContext);

  returnCode = mbedtls_ctr_drbg_seed(&ctrDrbg, mbedtls_entropy_func, &entropy,
                                     (uint8_t *)custom, strlen(custom));
  ESP_LOGI(TAG, "mbedtls_ctr_drbg_seed return code: %d", returnCode);

  returnCode = mbedtls_ecdsa_from_keypair(&context, keyPair);
  ESP_LOGI(TAG, "mbedtls_ecdsa_from_keypair return code: %d", returnCode);

  auto digest = computeSha384(message, messageLength);

  std::unique_ptr<uint8_t[]> result(new uint8_t[80]);
  returnCode = mbedtls_ecdsa_write_signature(
      &context, MBEDTLS_MD_SHA384, digest.get(), 64, result.get(),
      &signatureLength, mbedtls_ctr_drbg_random, &ctrDrbg);
  ESP_LOGI(TAG, "mbedtls_ecdsa_write_signature return code: %d", returnCode);
  ESP_LOGI(TAG, "Signature length: %d", signatureLength);

  mbedtls_ecdsa_free(&context);
  mbedtls_ctr_drbg_free(&ctrDrbg);
  mbedtls_entropy_free(&entropy);
  mbedtls_pk_free(&pkContext);

  return std::move(result);
}

bool verifyEcdsa(uint8_t *message, size_t messageLength, uint8_t *signature,
                 size_t                   signatureLength,
                 const mbedtls_ecp_point &peerPublicParam)
{
  ESP_LOGI(TAG, "Verifying ECDSA signature");

  mbedtls_ecdsa_context context;

  mbedtls_ecdsa_init(&context);

  auto returnCode =
      mbedtls_ecp_group_load(&context.grp, MBEDTLS_ECP_DP_BP256R1);
  ESP_LOGI(TAG, "mbedtls_ecp_group_load return code: %d", returnCode);

  returnCode = mbedtls_ecp_copy(
      &context.Q, &peerPublicParam); // only the public part is needed
  ESP_LOGI(TAG, "mbedtls_ecp_copy return code: %d", returnCode);

  auto digest = computeSha384(message, messageLength);

  returnCode = mbedtls_ecdsa_read_signature(&context, digest.get(), 64,
                                            signature, signatureLength);
  ESP_LOGI(TAG, "mbedtls_ecdsa_read_signature return code: %d", returnCode);

  mbedtls_ecdsa_free(&context);

  return returnCode == 0;
}
} // end namespace crypto