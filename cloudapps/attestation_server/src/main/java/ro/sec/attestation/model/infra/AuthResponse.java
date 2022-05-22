package ro.sec.attestation.model.infra;

import lombok.Data;

@Data
public class AuthResponse {

    private String access_token;
    private String token_type;
}
