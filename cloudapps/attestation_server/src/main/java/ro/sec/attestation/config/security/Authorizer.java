package ro.sec.attestation.config.security;

import org.springframework.beans.factory.annotation.Value;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;
import org.springframework.http.HttpEntity;
import org.springframework.http.HttpHeaders;
import org.springframework.http.HttpMethod;
import org.springframework.web.client.RestTemplate;
import ro.sec.attestation.model.infra.AuthRequest;
import ro.sec.attestation.model.infra.AuthResponse;
import ro.sec.attestation.model.infra.VaultProperties;
import ro.sec.attestation.model.infra.VaultResponse;

@Configuration
public class Authorizer {

    private static final String TOKEN_ENDPOINT = "oauth/token";
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
        var response = restTemplate
                .postForObject(issuerUrl + TOKEN_ENDPOINT, requestBody, AuthResponse.class);

        return response
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
