package ro.dissertation.dbapp.web.dto;

import lombok.Data;

@Data
public class IotResourceUpdateDto {

    private int resourceId;
    private String friendlyName;
}
