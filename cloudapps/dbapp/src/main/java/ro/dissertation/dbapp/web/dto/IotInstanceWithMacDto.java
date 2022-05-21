package ro.dissertation.dbapp.web.dto;

import lombok.Data;

@Data
public class IotInstanceWithMacDto {

    private String macAddress;
    private int instanceId;
}
