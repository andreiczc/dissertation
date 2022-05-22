package ro.sec.attestation.model;

import lombok.Data;

import java.util.List;

@Data
public class IotObject {

    private int objectId;
    private List<IotResource> resources;
}
