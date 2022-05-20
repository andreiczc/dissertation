package ro.dissertation.dbapp.config;

import org.springframework.beans.factory.annotation.Value;
import org.springframework.boot.jdbc.DataSourceBuilder;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;
import org.springframework.transaction.annotation.EnableTransactionManagement;
import ro.dissertation.dbapp.model.VaultProperties;

import javax.sql.DataSource;

@EnableTransactionManagement
@Configuration
public class DatabaseConfiguration {

    @Value("${spring.datasource.url}")
    private String dbUrl;

    @Bean
    public DataSource getDataSource(VaultProperties vaultProperties) {
        var builder = DataSourceBuilder.create();
        builder.url(dbUrl);
        builder.username(vaultProperties.getDbUser());
        builder.password(vaultProperties.getDbPass());

        return builder.build();
    }
}
