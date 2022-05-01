#include "coap_client.h"
#include "crypto_utils.h"
#include "esp_event.h"
#include "esp_log.h"
#include "net_utils.h"
#include "nvs_flash.h"

static constexpr const auto *TAG      = "MAIN";
static constexpr const auto  CRT_SIZE = 555;
static constexpr const auto  KEY_SIZE = 122;

extern const uint8_t device_crt_start[] asm("_binary_device_crt_start");
extern const uint8_t device_crt_end[] asm("_binary_device_crt_end");
extern const uint8_t device_key_start[] asm("_binary_device_key_start");
extern const uint8_t device_key_end[] asm("_binary_device_key_end");
extern const uint8_t wallet_key_start[] asm("_binary_wallet_key_start");
extern const uint8_t wallet_key_end[] asm("_binary_wallet_key_end");

static void init()
{
  auto ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
  {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);

  ESP_ERROR_CHECK(esp_event_loop_create_default());
}

extern "C" void app_main()
{
  ESP_LOGI(TAG, "Starting node...");
  init();
  ESP_LOGI(TAG, "Init complete");

  /* const auto netUtils = NetUtils::getInstance();
  netUtils->startWifi();

  // const auto client = netUtils->initMqttConnection();
  CoapClient client("192.168.0.180", 5683);

  size_t payloadLength;
  auto  *payload = client.doGet(payloadLength, "test"); */

  /* const auto plaintext = "hello";
  uint8_t    key[]     = {
      0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x0, 0x1, 0x2,
      0x3, 0x4, 0x5, 0x6, 0x7, 0x0, 0x1, 0x2, 0x3, 0x4, 0x5,
      0x6, 0x7, 0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7,
  };

  uint8_t    iv1[]      = {0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7,
                   0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7};
  uint16_t   outputSize = 0;
  const auto cipherText =
      crypto::encryptAes((uint8_t *)plaintext, key, iv1, outputSize);

  uint8_t    iv2[]    = {0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7,
                   0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7};
  const auto restored = crypto::decryptAes(cipherText.get(), 16, key, iv2);

  ESP_LOGI(TAG, "Restored: %s", restored.get()); */

  /* ESP_LOGI(TAG, "Calculating 1st param pair");
  auto ctx1 = crypto::generateEcdhParams();
  ESP_LOGI(TAG, "Finished 1st param pair");

  mbedtls_ecp_point point;
  mbedtls_ecp_point_init(&point);

  mbedtls_mpi_init(&point.X);
  mbedtls_mpi_init(&point.Y);
  mbedtls_mpi_init(&point.Z);

  uint8_t ptX[32] = {0x0e, 0xdd, 0xa5, 0xe3, 0xbd, 0x71, 0x4e, 0x76,
                     0xc3, 0x6b, 0xee, 0x70, 0x13, 0x05, 0xc0, 0x7e,
                     0x59, 0xdf, 0x35, 0x4e, 0x78, 0x46, 0xa3, 0xf8,
                     0x43, 0x1b, 0xfa, 0xf8, 0x69, 0x26, 0x87, 0x5e};
  mbedtls_mpi_read_binary(&point.X, ptX, 32);

  uint8_t ptY[32] = {0x25, 0xa6, 0x72, 0xa8, 0x98, 0x23, 0x54, 0xa0,
                     0x19, 0xe9, 0x4f, 0x15, 0xa6, 0x20, 0x27, 0xc9,
                     0x24, 0xf9, 0x05, 0xee, 0x8c, 0x70, 0x66, 0x9d,
                     0x0e, 0x24, 0x11, 0x80, 0x7a, 0x6d, 0x43, 0x9e};
  mbedtls_mpi_read_binary(&point.Y, ptY, 32);

  mbedtls_mpi_lset(&point.Z, 1);

  auto generated1 = crypto::generateSharedSecret(ctx1, point); */

  uint8_t   *message       = (uint8_t *)"hello";
  const auto messageLength = 4;

  size_t     signatureLength = 0;
  const auto signature       = crypto::signEcdsa(
            message, messageLength, signatureLength, wallet_key_start, KEY_SIZE);
  ESP_LOG_BUFFER_HEX(TAG, signature.get(), signatureLength);

  /* const auto verifies =
      crypto::verifyEcdsa(message, messageLength, signature.get(),
                          signatureLength, device_crt_start, CRT_SIZE);
  ESP_LOGI(TAG, "Signatures verfies: %s", verifies ? "YES" : "NO"); */

  /* const auto netUtils = NetUtils::getInstance();
  netUtils->startWifi();

  const auto functionSignature = "insertEntry(string,uint256)";
  const auto digest            = crypto::keccak256((uint8_t *)functionSignature,
                                                   strlen(functionSignature));
*/
}