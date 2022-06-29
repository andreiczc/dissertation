package ro.sec.attestation.web.dto;

import lombok.Data;

@Data
public class AuthResponse {

    private String access_token;
    private String token_type;
}
