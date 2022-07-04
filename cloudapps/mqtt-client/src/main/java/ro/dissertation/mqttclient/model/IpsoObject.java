package ro.dissertation.mqttclient.model;

import lombok.Data;

import java.util.List;

@Data
public class IpsoObject {

    private long timestamp;
    private List<IpsoValue> values;
}
