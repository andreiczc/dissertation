#include "mqtt_client.h"

MqttClient::MqttClient(
    const String &clientId, const String &brokerIp, int port,
    const String                                        &caCert,
    std::function<void(char *, uint8_t *, unsigned int)> callback)
    : client(this->wifiClient)
{
  wifiClient.setCACert(caCert.c_str());

  client.setServer(brokerIp.c_str(), port)
      .setCallback(callback)
      .connect(clientId.c_str());

  if (client.connected() != MQTT_CONNECTED)
  {
    log_e("Error while connecting to broker. Will restart in 5 secs...");
    delay(5000);
    ESP.restart();
  }
}

boolean MqttClient::subscribe(const String &topic, unsigned char qos)
{
  return this->client.subscribe(topic.c_str(), qos);
}

boolean MqttClient::publish(const String &topic, const String &payload,
                            boolean retained)
{
  return this->client.publish(topic.c_str(), (const uint8_t *)payload.c_str(),
                              payload.length(), retained);
}

MqttClient MqttClientBuilder::build() {}