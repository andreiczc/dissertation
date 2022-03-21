package ro.ase.clientsecure;

import org.eclipse.paho.client.mqttv3.*;
import org.eclipse.paho.client.mqttv3.persist.MemoryPersistence;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

@Component
public class MyClient {

    private static final Logger logger = LoggerFactory.getLogger(MyClient.class);

    private static final String broker = "ssl://130.162.253.10:8883";
    private static final String clientId = "TestClient";
    private static final String topic = "TestTopic";

    private MqttClientPersistence persistence;
    private MqttClient client;

    @Autowired
    public MyClient() {
        try {
            this.persistence = new MemoryPersistence();
            client = new MqttClient(broker, clientId, persistence);
            var connOpts = new MqttConnectOptions();
            connOpts.setCleanSession(true);
            client.connect(connOpts);
            var message = new MqttMessage("hello".getBytes());
            message.setQos(0);
            client.publish(topic, message);
        } catch (MqttException e) {
            e.printStackTrace();
        }
    }

}
