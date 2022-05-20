package ro.dissertation.mqttclient.model;

import lombok.Data;

import java.util.List;

@Data
public class IpsoObject {

    private int timestamp;
    private List<IpsoValue> values;
}
