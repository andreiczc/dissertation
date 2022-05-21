package ro.dissertation.dbapp.web.dto;

import lombok.Data;

@Data
public class IotObjectUpdateDto {

    private int objectId;
    private String friendlyName;
}
