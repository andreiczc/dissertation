package ro.dissertation.mqttclient.model;

import lombok.Data;

@Data
public class IpsoValue {

    private int objectId;
    private int instanceId;
    private int resourceId;
    private String datatype;
    private String value;
}
