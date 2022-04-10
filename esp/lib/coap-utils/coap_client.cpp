#include "coap_client.h"

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"

#include <cstdio>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "esp_event.h"
#include "esp_log.h"
#include "esp_wifi.h"

static const auto *TAG = "MQTT_CLIENT";

static uint8_t *payload = nullptr;
static size_t   payloadLength;

static void safeDelete(uint8_t **buffer)
{
  if (*buffer == nullptr)
  {
    return;
  }

  free(*buffer);
  *buffer = nullptr;
}

static void handleFailure(const char *message)
{
  ESP_LOGE(TAG, "%s", message);
  vTaskDelay(5000 / portTICK_PERIOD_MS);
  esp_restart();
}

static void coapMessageHandler(struct coap_context_t *context,
                               coap_session_t *session, coap_pdu_t *sent,
                               coap_pdu_t *received, const coap_tid_t id)
{
  ESP_LOGI(TAG, "Message with token id %d has been received", received->tid);
  if (COAP_RESPONSE_CLASS(received->code) != COAP_RESPONSE_304)
  {
    handleFailure("The message doesn't have VALID response code");
  }

  coap_get_data(received, &payloadLength, &payload);
}

static int resolve_address(const char *host, const char *service,
                           coap_address_t *dst)
{

  struct addrinfo *res, *ainfo;
  struct addrinfo  hints;
  int              error, len = -1;

  memset(&hints, 0, sizeof(hints));
  memset(dst, 0, sizeof(*dst));
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_family   = AF_UNSPEC;

  error = getaddrinfo(host, service, &hints, &res);

  if (error)
  {
    handleFailure("Couldn't get address info!");
    return error;
  }

  for (ainfo = res; ainfo != NULL; ainfo = ainfo->ai_next)
  {
    switch (ainfo->ai_family)
    {
    case AF_INET6:
    case AF_INET:
      len = dst->size = ainfo->ai_addrlen;
      memcpy(&dst->addr.sin6, ainfo->ai_addr, dst->size);
      goto finish;
    default:;
    }
  }

finish:
  freeaddrinfo(res);
  return len;
}

CoapClient::CoapClient(const std::string &serverUrl, int port)
{
  coap_set_log_level(LOG_INFO);

  coap_address_t dst;

  if (resolve_address(serverUrl.c_str(), std::to_string(port).c_str(), &dst))
  {
    handleFailure("Couldn't get address info!");
  }

  this->context = coap_new_context(nullptr);
  if (!this->context || !(this->session = coap_new_client_session(
                              this->context, nullptr, &dst, COAP_PROTO_UDP)))
  {
    handleFailure("Coap client not acquired... Will restart in 5 seconds");
  }

  coap_register_response_handler(context, coapMessageHandler);
}

CoapClient::~CoapClient()
{
  coap_session_release(this->session);
  coap_free_context(this->context);
  coap_cleanup();
}

uint8_t *CoapClient::doGet(size_t &lengthReceived, const std::string &uriPath)
{
  auto *pdu = coap_pdu_init(COAP_MESSAGE_CON, COAP_REQUEST_GET,
                            coap_new_message_id(this->session),
                            coap_session_max_pdu_size(this->session));

  if (!pdu)
  {
    handleFailure("Couldn't initialize PDU!");
  }

  coap_add_option(pdu, COAP_OPTION_URI_PATH, uriPath.length(),
                  (const uint8_t *)uriPath.c_str());

  safeDelete(&payload);

  coap_send(session, pdu);

  auto retryCounter = 12;
  while (!payload || !retryCounter)
  {
    vTaskDelay(5000 / portTICK_PERIOD_MS);
    --retryCounter;
  }

  lengthReceived = payloadLength;

  return payload;
}

uint8_t *CoapClient::doPost(const std::string &body, size_t &lengthReceived,
                            const std::string &uriPath)
{
  auto *pdu = coap_pdu_init(COAP_MESSAGE_CON, COAP_REQUEST_POST,
                            coap_new_message_id(this->session),
                            coap_session_max_pdu_size(this->session));

  if (!pdu)
  {
    handleFailure("Couldn't initialize PDU!");
  }

  coap_add_option(pdu, COAP_OPTION_URI_PATH, uriPath.length(),
                  (const uint8_t *)uriPath.c_str());
  coap_add_data(pdu, body.length(), (const uint8_t *)body.c_str());

  safeDelete(&payload);

  coap_send(session, pdu);

  auto retryCounter = 12;
  while (!payload || !retryCounter)
  {
    vTaskDelay(5000 / portTICK_PERIOD_MS);
    --retryCounter;
  }

  lengthReceived = payloadLength;

  return payload;
}
