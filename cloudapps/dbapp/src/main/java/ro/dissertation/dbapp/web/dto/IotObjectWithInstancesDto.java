package ro.dissertation.dbapp.web.dto;

import lombok.Data;

import java.util.List;

@Data
public class IotObjectWithInstancesDto {

    private int objectId;

    private List<IotInstanceWithMacDto> instances;
}
