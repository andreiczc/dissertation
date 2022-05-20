package ro.dissertation.mqttclient.model;

import lombok.AllArgsConstructor;
import lombok.Data;

@Data
@AllArgsConstructor
public class AuthRequest {

    private String client_id;
    private String client_secret;
    private String audience;
    private String grant_type;
}
