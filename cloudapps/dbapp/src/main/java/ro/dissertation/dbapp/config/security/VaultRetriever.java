package ro.dissertation.dbapp.config.security;

import org.springframework.beans.factory.annotation.Value;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;
import org.springframework.http.HttpEntity;
import org.springframework.http.HttpHeaders;
import org.springframework.http.HttpMethod;
import org.springframework.web.client.RestTemplate;
import ro.dissertation.dbapp.model.VaultProperties;
import ro.dissertation.dbapp.model.VaultResponse;

@Configuration
public class VaultRetriever {

    private static final String VAULT_ENDPOINT = "v1/sys/wrapping/unwrap";
    private static final String VAULT_HEADER = "X-Vault-Token";

    @Value("${vault.uri}")
    private String vaultUri;
    @Value("${vault.token}")
    private String vaultToken;

    @Bean
    public VaultProperties vaultProperties(RestTemplate restTemplate) {
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
