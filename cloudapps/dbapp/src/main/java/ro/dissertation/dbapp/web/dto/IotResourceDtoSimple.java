package ro.dissertation.dbapp.web.dto;

import lombok.Data;

import javax.validation.constraints.Min;

@Data
public class IotResourceDtoSimple {

    @Min(2000)
    private int resourceId;
}
