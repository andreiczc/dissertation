package ro.dissertation.dbapp.web.dto;

import lombok.Data;

import javax.validation.constraints.Pattern;

@Data
public class MachineUpdateDto {

    @Pattern(regexp = "^([0-9A-Fa-f]{2}[:-]){5}([0-9A-Fa-f]{2})|([0-9a-fA-F]{4}\\\\.[0-9a-fA-F]{4}\\\\.[0-9a-fA-F]{4})$")
    private String macAddress;

    private String friendlyName;
}
