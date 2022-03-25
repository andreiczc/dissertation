package ro.sec.coap;

import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NoArgsConstructor;

@Data
@AllArgsConstructor
@NoArgsConstructor
public class ClientPayload {

    private byte[] publicParams;
    private byte[] signature;
}
