package ro.sec.attestation.web;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RestController;
import ro.sec.attestation.service.api.AttestationService;
import ro.sec.attestation.web.dto.ClientPayload;
import ro.sec.attestation.web.dto.MachineIdentifierResponseDto;
import ro.sec.attestation.web.dto.ServerPayload;

@RestController
@RequestMapping("/attestation")
public class Controller {

    // TODO extract these variables to smth common

    private static final String CLIENT_HELLO_RSC = "/clientHello";
    private static final String KEY_EXCHANGE_RSC = "/keyExchange";
    private static final String CLIENT_FINISHED_RSC = "/clientFinished";

    private final AttestationService service;

    @Autowired
    public Controller(AttestationService service) {
        this.service = service;
    }

    @PostMapping(CLIENT_HELLO_RSC)
    public ResponseEntity<byte[]> clientHello(@RequestBody String payload) throws Exception {
        return ResponseEntity.ok().body(service.clientHello(payload));
    }

    @PostMapping(KEY_EXCHANGE_RSC)
    public ResponseEntity<ServerPayload> keyExchange(@RequestBody ClientPayload payload) throws Exception {
        return ResponseEntity.ok().body(service.keyExchange(payload));
    }

    @PostMapping(CLIENT_FINISHED_RSC)
    public ResponseEntity<MachineIdentifierResponseDto> clientFinished(@RequestBody String payload) throws Exception {
        return ResponseEntity
                .ok()
                .body(service.clientFinish(payload));
    }
}
