package ro.dissertation.mqttclient.model;

import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NoArgsConstructor;

import java.util.Date;

@Data
@AllArgsConstructor
@NoArgsConstructor
public class IotRecord {

    private long id;
    private String datatype;
    private String value;
    private Date timestamp;
    private IotResource resource;

    public IotRecord(IpsoValue value, Date timestamp) {
        this.datatype = value.getDatatype();
        this.value = value.getValue();
        this.timestamp = timestamp;
        this.resource = new IotResource(value.getResourceId());
    }
}
