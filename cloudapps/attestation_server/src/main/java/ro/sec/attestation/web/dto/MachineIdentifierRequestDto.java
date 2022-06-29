package ro.sec.attestation.web.dto;

import lombok.Data;

import java.util.Date;
import java.util.List;

@Data
public class MachineIdentifierRequestDto {

    private String macAddress;
    private List<IotObject> objects;
    private Date lastAttestation;

    public MachineIdentifierRequestDto(MachineIdentifier machineIdentifier) {
        this.macAddress = machineIdentifier.getMacAddress();
        this.objects = machineIdentifier.getObjects();
        this.lastAttestation = new Date();
    }
}
