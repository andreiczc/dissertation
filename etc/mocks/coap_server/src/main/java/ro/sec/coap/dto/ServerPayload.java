package ro.sec.coap.dto;

import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NoArgsConstructor;

@Data
@AllArgsConstructor
@NoArgsConstructor
public class ServerPayload {

    private byte[] publicParams;
    private byte[] signature;
    private byte[] test;
}
