package ro.dissertation.mqttclient.service;

import org.springframework.stereotype.Component;
import ro.dissertation.mqttclient.config.security.Authorizer;

@Component
public class RecordService {

    private final Authorizer authorizer;

    public RecordService(Authorizer authorizer) {
        this.authorizer = authorizer;
    }
}
