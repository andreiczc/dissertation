package ro.dissertation.mqttclient.config.security;

import org.springframework.beans.factory.annotation.Value;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;
import org.springframework.http.HttpEntity;
import org.springframework.http.HttpHeaders;
import org.springframework.http.HttpMethod;
import org.springframework.http.MediaType;
import org.springframework.web.client.RestTemplate;
import ro.dissertation.mqttclient.model.AuthRequest;
import ro.dissertation.mqttclient.model.AuthResponse;
import ro.dissertation.mqttclient.model.VaultProperties;
import ro.dissertation.mqttclient.model.VaultResponse;

@Configuration
public class Authorizer {

    private static final String TOKEN_ENDPOINT = "ouath/token";
    private static final String VAULT_ENDPOINT = "v1/sys/wrapping/unwrap";
    private static final String VAULT_HEADER = "X-Vault-Token";

    @Value("${auth0.issuer}")
    private String issuerUrl;
    @Value("${auth0.client-id}")
    private String clientId;
    @Value("${auth0.audience}")
    private String audience;
    @Value("${auth0.grant-type}")
    private String grantType;
    @Value("${vault.uri}")
    private String vaultUri;
    @Value("${vault.token}")
    private String vaultToken;

    @Bean
    public String bearerToken(RestTemplate restTemplate) {
        var secrets = getSecrets(restTemplate);
        var requestBody = new AuthRequest(clientId, secrets.getClientSecret(), audience, grantType);
        var headers = new HttpHeaders();
        headers.setContentType(MediaType.APPLICATION_JSON);
        var entity = new HttpEntity<>(requestBody, headers);
        var response = restTemplate
                .exchange(issuerUrl + TOKEN_ENDPOINT,
                        HttpMethod.POST,
                        entity,
                        AuthResponse.class);


        return response
                .getBody()
                .getAccess_token();
    }

    private VaultProperties getSecrets(RestTemplate restTemplate) {
        var headers = new HttpHeaders();
        headers.set(VAULT_HEADER, vaultToken);
        var entity = new HttpEntity<>(null, headers);
        var response = restTemplate
                .exchange(vaultUri + VAULT_ENDPOINT,
                        HttpMethod.POST,
                        entity,
                        VaultResponse.class);

        return response
                .getBody()
                .getData();
    }
}
