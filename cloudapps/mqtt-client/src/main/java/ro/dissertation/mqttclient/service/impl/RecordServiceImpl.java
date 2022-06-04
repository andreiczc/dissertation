package ro.dissertation.mqttclient.service.impl;

import org.springframework.beans.factory.annotation.Value;
import org.springframework.http.HttpEntity;
import org.springframework.http.HttpHeaders;
import org.springframework.http.HttpMethod;
import org.springframework.stereotype.Component;
import org.springframework.web.client.RestTemplate;
import ro.dissertation.mqttclient.model.IotRecord;
import ro.dissertation.mqttclient.model.IpsoValue;
import ro.dissertation.mqttclient.service.api.RecordService;

import java.util.Date;

@Component
public class RecordServiceImpl implements RecordService {

    private static final String AUTHORIZATION_HEADER = "Authorization";
    private static final String AUTHORIZATION_PATTERN = "Bearer %s";
    private static final String ENDPOINT = "/iot/record";

    @Value("${gw.uri}")
    private String gwUri;
    private final RestTemplate restTemplate;
    private final String bearerToken;

    public RecordServiceImpl(RestTemplate restTemplate, String bearerToken) {
        this.restTemplate = restTemplate;
        this.bearerToken = bearerToken;
    }

    public IotRecord persistRecord(IpsoValue object, Date timestamp) {
        var headers = new HttpHeaders();
        headers.set(AUTHORIZATION_HEADER, String.format(AUTHORIZATION_PATTERN, bearerToken));
        var entity = new HttpEntity<>(new IotRecord(object, timestamp), headers);
        var response = restTemplate
                .exchange(gwUri + ENDPOINT,
                        HttpMethod.POST,
                        entity,
                        IotRecord.class);

        return response
                .getBody();
    }
}
