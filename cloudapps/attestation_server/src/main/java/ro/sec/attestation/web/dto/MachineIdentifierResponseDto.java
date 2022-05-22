package ro.sec.attestation.web.dto;

import lombok.Data;

import java.util.List;

@Data
public class MachineIdentifierResponseDto {

    private String macAddress;
    private List<IotObjectWithInstanceDto> objects;
}
