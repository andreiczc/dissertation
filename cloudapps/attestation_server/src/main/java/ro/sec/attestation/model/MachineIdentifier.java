package ro.sec.attestation.model;

import lombok.Data;

import java.util.List;

@Data
public class MachineIdentifier {

    private String macAddress;
    private List<IotObject> objects;
}
