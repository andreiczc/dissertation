package ro.sec.attestation.service.impl;

import com.fasterxml.jackson.databind.ObjectMapper;
import org.bouncycastle.jcajce.provider.asymmetric.ec.BCECPublicKey;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.http.HttpEntity;
import org.springframework.http.HttpHeaders;
import org.springframework.http.HttpMethod;
import org.springframework.stereotype.Service;
import org.springframework.web.client.RestTemplate;
import org.springframework.web.context.request.RequestContextHolder;
import org.springframework.web.context.request.ServletRequestAttributes;
import ro.sec.attestation.Application;
import ro.sec.attestation.model.MachineIdentifier;
import ro.sec.attestation.repo.SecureStore;
import ro.sec.attestation.service.api.AttestationService;
import ro.sec.attestation.service.exception.BadSignatureException;
import ro.sec.attestation.service.exception.BadTestBytesException;
import ro.sec.attestation.web.dto.ClientPayload;
import ro.sec.attestation.web.dto.MachineIdentifierRequestDto;
import ro.sec.attestation.web.dto.MachineIdentifierResponseDto;
import ro.sec.attestation.web.dto.ServerPayload;
import ro.sec.crypto.CryptoUtils;

import javax.crypto.SecretKey;
import java.security.PrivateKey;
import java.security.cert.Certificate;
import java.security.cert.CertificateFactory;
import java.security.interfaces.ECPublicKey;
import java.util.Arrays;
import java.util.Base64;
import java.util.HashMap;
import java.util.Map;

@Service
public class AttestationServiceImpl implements AttestationService {

    private static final String AUTHORIZATION_HEADER = "Authorization";
    private static final String AUTHORIZATION_PATTERN = "Bearer %s";
    private static final String ENDPOINT = "/iot/upsert";
    private static final Logger log = LoggerFactory.getLogger(AttestationServiceImpl.class);
    private static final int TEST_BYTES_LENGTH = 16;

    @Value("${gw.uri}")
    private String GW_URL;
    private final Map<String, SecretKey> secretStore;
    private final SecureStore pskStore;
    private final Certificate certificate;
    private final PrivateKey privateKey;
    private final Map<String, Certificate> certificateMap;
    private final Map<String, byte[]> testBytesMap;
    private final String bearerToken;
    private final RestTemplate restTemplate;

    @Autowired
    public AttestationServiceImpl(SecureStore pskStore, String bearerToken, RestTemplate restTemplate) throws Exception {
        this.restTemplate = restTemplate;
        try (var ownCertificateInputStream = Application.class.getClassLoader().getResourceAsStream("server.crt");
             var privateKeyInputStream = Application.class.getClassLoader().getResourceAsStream("server.key")) {
            this.certificate = CertificateFactory
                    .getInstance("X.509")
                    .generateCertificate(ownCertificateInputStream);
            var privateKeyBytes = privateKeyInputStream.readAllBytes();
            this.privateKey = CryptoUtils.readPrivateKey(privateKeyBytes);
        }

        this.certificateMap = new HashMap<>();
        this.testBytesMap = new HashMap<>();
        this.secretStore = new HashMap<>();
        this.pskStore = pskStore;
        this.bearerToken = bearerToken;
    }

    @Override
    public byte[] clientHello(String clientCertificate) throws Exception {
        var certificateDecoded = Base64.getDecoder().decode(clientCertificate);
        var certificate = CryptoUtils.readCertificate(certificateDecoded);

        var remoteIp = getRequestIp();

        certificateMap.put(
                remoteIp,
                certificate
        );

        var x = ((ECPublicKey) this.certificate.getPublicKey()).getW().getAffineX().toByteArray();
        x = Arrays.copyOfRange(x, 1, x.length);
        var y = ((ECPublicKey) this.certificate.getPublicKey()).getW().getAffineY().toByteArray();

        var result = Arrays.copyOf(x, x.length + y.length);
        System.arraycopy(y, 0, result, x.length, y.length);
        return Base64.getEncoder().encode(result);
    }

    @Override
    public ServerPayload keyExchange(ClientPayload clientPayload) throws Exception {
        var clientAddress = getRequestIp();

        var thirdPartyCertificate = certificateMap.get(clientAddress);

        var base64Decoder = Base64.getDecoder();
        var publicParams = base64Decoder.decode(clientPayload.getPublicParams());
        var signature = base64Decoder.decode(clientPayload.getSignature());

        var clientSignatureVerifies = CryptoUtils.verifyEcdsa(
                publicParams,
                signature,
                thirdPartyCertificate.getPublicKey()
        );

        if (!clientSignatureVerifies) {
            log.info("Signature from {} didn't verify!", clientAddress);

            throw new BadSignatureException();
        }

        log.info("Signature from {} verifies", clientAddress);

        var ownKeyPair = CryptoUtils.generateEcKeyPair();
        var thirdPartyPublicKey = CryptoUtils.decodePublicPoint(publicParams, ((BCECPublicKey) ownKeyPair.getPublic()).getParams());
        var publicKeyPoint = CryptoUtils.packagePublicEcPoint((BCECPublicKey) ownKeyPair.getPublic());
        var signatureOfPublicKey = CryptoUtils
                .signEcdsa(publicKeyPoint, privateKey);

        var testBytes = CryptoUtils.generateRandomSequence(TEST_BYTES_LENGTH);
        testBytesMap.put(clientAddress, testBytes);

        var base64Encoder = Base64.getEncoder();

        var serverPayload = new ServerPayload(
                base64Encoder.encodeToString(publicKeyPoint),
                base64Encoder.encodeToString(signatureOfPublicKey),
                base64Encoder.encodeToString(testBytes));

        var sharedSecret = CryptoUtils
                .generateSharedSecret(ownKeyPair.getPrivate(), thirdPartyPublicKey);
        log.info("Shared secret for {} was generated successfully", clientAddress);
        secretStore.put(clientAddress, sharedSecret);

        return serverPayload;
    }

    @Override
    public MachineIdentifierResponseDto clientFinish(String encodedPayload) throws Exception {
        var payload = Base64.getDecoder().decode(encodedPayload);
        var clientAddress = getRequestIp();

        var iv = Arrays.copyOfRange(payload, 0, 16);
        var ciphertext = Arrays.copyOfRange(payload, 16, payload.length);
        var sessionKey = secretStore.remove(clientAddress);

        var secretBytes = testBytesMap.remove(clientAddress);
        var decrypted = CryptoUtils.decryptAes(ciphertext, iv, sessionKey);

        var decryptedTest = Arrays.copyOfRange(decrypted, 0, 16);

        if (!Arrays.equals(secretBytes, decryptedTest)) {
            log.info("Payload from {} wasn't correct", clientAddress);

            throw new BadTestBytesException();
        }

        pskStore.store(clientAddress, sessionKey);
        log.info("Session with {} has been established successfully", clientAddress);

        var decryptedInfo = Arrays.copyOfRange(decrypted, 16, decrypted.length);
        var mappedObject = new ObjectMapper().readValue(decryptedInfo, MachineIdentifier.class);
        log.info("Received value: {}", mappedObject.toString());

        return issueRequest(mappedObject);
    }

    private MachineIdentifierResponseDto issueRequest(MachineIdentifier payload) {
        var headers = new HttpHeaders();
        headers.set(AUTHORIZATION_HEADER, String.format(AUTHORIZATION_PATTERN, bearerToken));
        var entity = new HttpEntity<>(new MachineIdentifierRequestDto(payload), headers);
        var response = restTemplate
                .exchange(GW_URL + ENDPOINT,
                        HttpMethod.POST,
                        entity,
                        MachineIdentifierResponseDto.class);

        return response
                .getBody();
    }


    private String getRequestIp() {
        return ((ServletRequestAttributes) RequestContextHolder.currentRequestAttributes())
                .getRequest()
                .getRemoteAddr();
    }
}
