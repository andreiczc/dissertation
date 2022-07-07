package ro.sec.attestation.web.dto;

import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NoArgsConstructor;

@Data
@AllArgsConstructor
@NoArgsConstructor
public class ServerPayload {

    private String publicParams;
    private String signature;
    private String test;
}
