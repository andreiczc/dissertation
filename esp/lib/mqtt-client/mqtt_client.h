#ifndef _MQTT_CLIENT_H
#define _MQTT_CLIENT_H

#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include <functional>
#include <memory>

class MqttClientBuilder;

class MqttClient
{
public:
  boolean subscribe(const String &topic, unsigned char qos = 0);

  boolean publish(const String &topic, const String &payload,
                  boolean retained = 0);

  MqttClient(const MqttClient &another) = delete;

  MqttClient &operator=(const MqttClient &another) = delete;

  MqttClient(const MqttClient &&another) = delete;

  MqttClient &operator=(const MqttClient &&another) = delete;

private:
  explicit MqttClient(
      const String &clientId, const String &brokerIp, int port,
      const String                                        &caCert,
      std::function<void(char *, uint8_t *, unsigned int)> callback);

  WiFiClientSecure wifiClient;
  PubSubClient     client;

  friend class MqttClientBuilder;
};

class MqttClientBuilder
{
public:
  explicit MqttClientBuilder(String clientId, String brokerIp)
      : clientId(clientId), brokerIp(brokerIp){};

  MqttClient build();

  MqttClientBuilder &setPort(int port) { this->port = port; }

  MqttClientBuilder &setCaCert(const String &caCert) { this->caCert = caCert; }

  MqttClientBuilder &
  setCallback(std::function<void(char *, uint8_t *, unsigned int)> callback)
  {
    this->callback = callback;
  }

private:
  String                                               clientId;
  String                                               brokerIp;
  int                                                  port     = 0;
  String                                               caCert   = "";
  std::function<void(char *, uint8_t *, unsigned int)> callback = nullptr;
};

#endif // _MQTT_CLIENT_H