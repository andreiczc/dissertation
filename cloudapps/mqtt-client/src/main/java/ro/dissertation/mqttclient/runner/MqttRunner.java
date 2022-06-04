package ro.dissertation.mqttclient.runner;

import com.fasterxml.jackson.databind.ObjectMapper;
import org.eclipse.paho.client.mqttv3.IMqttClient;
import org.eclipse.paho.client.mqttv3.IMqttDeliveryToken;
import org.eclipse.paho.client.mqttv3.MqttCallback;
import org.eclipse.paho.client.mqttv3.MqttMessage;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.boot.CommandLineRunner;
import org.springframework.stereotype.Component;
import ro.dissertation.mqttclient.model.IpsoObject;
import ro.dissertation.mqttclient.service.api.RecordService;

import java.util.Date;

@Component
public class MqttRunner implements CommandLineRunner {

    private final Logger log = LoggerFactory.getLogger(MqttRunner.class);
    private final IMqttClient mqttClient;
    private final ObjectMapper objectMapper;
    private final RecordService recordService;

    public MqttRunner(IMqttClient mqttClient, ObjectMapper objectMapper, RecordService recordService) {
        this.mqttClient = mqttClient;
        this.objectMapper = objectMapper;
        this.recordService = recordService;
    }

    @Override
    public void run(String... args) throws Exception {
        mqttClient.setCallback(new MqttCallback() {
            @Override
            public void connectionLost(Throwable throwable) {
                log.error("Connection to MQTT broker lost. Cause: {}",
                        throwable.toString());
            }

            @Override
            public void messageArrived(String s, MqttMessage mqttMessage) throws Exception {
                var payloadBytes = mqttMessage.getPayload();
                var hex = bytesToHex(payloadBytes);

                log.info("Message arrived on topic {}", s);
                log.debug("Payload: {}", hex);

                var payload = objectMapper.readValue(payloadBytes, IpsoObject.class);
                log.info("Received at timestamp: {}", payload.getTimestamp());
                log.info("Received value: {}", payload.getValues().get(0).getValue());

                var timestamp = new Date(payload.getTimestamp());
                for(var value : payload.getValues()) {
                    log.info("Saving record for resource {}", value.getResourceId());
                    var saved = recordService.persistRecord(value, timestamp);
                    log.info("ID of saved item: {}", saved.getId());
                }
            }

            @Override
            public void deliveryComplete(IMqttDeliveryToken iMqttDeliveryToken) {
                log.info("Message {} was delivered successfully",
                        iMqttDeliveryToken.getMessageId());
            }
        });
        mqttClient.subscribe("#");
        while (true) {
            Thread.sleep(5000);
        }
    }

    private String bytesToHex(byte[] bytes) {
        var result = new StringBuilder();
        for(var b : bytes) {
            result.append(String.format("%02X ", b));
        }

        return result.toString();
    }
}
