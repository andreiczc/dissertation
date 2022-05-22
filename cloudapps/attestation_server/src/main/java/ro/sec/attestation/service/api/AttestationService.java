package ro.sec.attestation.service.api;

import ro.sec.attestation.web.dto.ClientPayload;
import ro.sec.attestation.web.dto.MachineIdentifierResponseDto;
import ro.sec.attestation.web.dto.ServerPayload;

public interface AttestationService {

    byte[] clientHello(String clientCertificate) throws Exception;

    ServerPayload keyExchange(ClientPayload clientPayload) throws Exception;

    MachineIdentifierResponseDto clientFinish(String encodedPayload) throws Exception;
}
