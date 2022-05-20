package ro.dissertation.dbapp.config;

import org.springframework.context.annotation.Bean;
import org.springframework.security.config.web.server.ServerHttpSecurity;
import org.springframework.security.web.server.SecurityWebFilterChain;

public class SecurityConfiguration {

    @Bean
    public SecurityWebFilterChain springSecurityFilterChain(ServerHttpSecurity http) throws Exception {
        http
                .authorizeExchange()
                .anyExchange()
                    .authenticated()
                .and()
                .oauth2ResourceServer()
                    .jwt();

        return http.build();
    }
}
