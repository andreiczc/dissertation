package ro.sec.attestation.web.dto;

import lombok.Data;

import java.util.List;

@Data
public class MachineIdentifier {

    private String macAddress;
    private List<IotObject> objects;
}
