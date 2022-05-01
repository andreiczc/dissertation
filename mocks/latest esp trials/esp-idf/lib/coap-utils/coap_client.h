#ifndef _COAP_CLIENT_H
#define _COAP_CLIENT_H

#include "coap.h"
#include <string>

class CoapClient
{
public:
  explicit CoapClient(const std::string &serverUrl, int port);

  virtual ~CoapClient() noexcept;

  CoapClient(const CoapClient &another) = delete;

  CoapClient &operator=(const CoapClient &another) = delete;

  CoapClient(const CoapClient &&another) = delete;

  CoapClient &operator=(const CoapClient &&another) = delete;

  uint8_t *doGet(size_t &lengthReceived, const std::string &uriPath);

  uint8_t *doPost(const std::string &body, size_t &lengthReceived,
                  const std::string &uriPath);

private:
  coap_context_t *context;
  coap_session_t *session;
};

#endif // _COAP_CLIENT_H