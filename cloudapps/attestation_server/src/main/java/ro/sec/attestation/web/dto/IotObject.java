package ro.sec.attestation.web.dto;

import lombok.Data;

import java.util.List;

@Data
public class IotObject {

    private int objectId;
    private List<IotResource> resources;
}
