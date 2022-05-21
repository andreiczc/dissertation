package ro.dissertation.dbapp.web.dto;

import lombok.Data;

import javax.validation.constraints.Min;
import javax.validation.constraints.NotNull;
import java.util.List;

@Data
public class IotObjectDto {

    @Min(1000)
    private int objectId;

    @NotNull
    private List<IotResourceDto> resources;
}
