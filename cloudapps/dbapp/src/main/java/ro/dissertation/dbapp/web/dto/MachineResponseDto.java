package ro.dissertation.dbapp.web.dto;

import lombok.Data;

import java.util.List;

@Data
public class MachineResponseDto {

    private String macAddress;

    private List<IotObjectResponseDto> objects;
}
