package ro.dissertation.dbapp.web.dto;

import lombok.Data;

import java.util.List;

@Data
public class IotResourceWithValuesDto {

    private int resourceId;
    private String friendlyName;
    private List<IotRecordDto> records;
}
