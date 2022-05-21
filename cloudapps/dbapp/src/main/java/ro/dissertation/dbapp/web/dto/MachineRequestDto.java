package ro.dissertation.dbapp.web.dto;

import lombok.Data;

import javax.validation.constraints.NotNull;
import javax.validation.constraints.Pattern;
import java.util.Date;
import java.util.List;

@Data
public class MachineRequestDto {

    @Pattern(regexp = "^([0-9A-Fa-f]{2}[:-]){5}([0-9A-Fa-f]{2})|([0-9a-fA-F]{4}\\\\.[0-9a-fA-F]{4}\\\\.[0-9a-fA-F]{4})$")
    private String macAddress;

    @NotNull
    private Date lastAttestation;

    @NotNull
    private List<IotObjectDto> objects;
}
