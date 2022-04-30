package ro.sec.attestation.service.impl;

import com.fasterxml.jackson.databind.ObjectMapper;
import org.bouncycastle.jcajce.provider.asymmetric.ec.BCECPublicKey;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;
import org.springframework.web.context.request.RequestContextHolder;
import org.springframework.web.context.request.ServletRequestAttributes;
import ro.sec.attestation.Application;
import ro.sec.attestation.repo.SecureStore;
import ro.sec.attestation.repo.SecureStoreInMemory;
import ro.sec.attestation.service.api.AttestationService;
import ro.sec.attestation.service.exception.BadSignatureException;
import ro.sec.attestation.service.exception.BadTestBytesException;
import ro.sec.attestation.web.dto.ClientPayload;
import ro.sec.attestation.web.dto.ServerPayload;
import ro.sec.crypto.CryptoUtils;

import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.nio.file.Files;
import java.security.PrivateKey;
import java.security.cert.Certificate;
import java.security.cert.CertificateFactory;
import java.util.Arrays;
import java.util.Base64;
import java.util.HashMap;
import java.util.Map;
import java.util.stream.Collectors;

@Service
public class AttestationServiceImpl implements AttestationService {

    private static final Logger log = LoggerFactory.getLogger(AttestationServiceImpl.class);
    private static final int TEST_BYTES_LENGTH = 16;

    private final SecureStore secretStore;
    private final SecureStore pskStore;
    private final String certificate;
    private final PrivateKey privateKey;
    private final Map<String, Certificate> certificateMap;
    private final Map<String, byte[]> testBytesMap;

    @Autowired
    public AttestationServiceImpl() throws Exception {
        try (var ownCertificateInputStream = new BufferedReader(new InputStreamReader(Application.class.getClassLoader().getResourceAsStream("server.crt")));
             var privateKeyInputStream = Application.class.getClassLoader().getResourceAsStream("server.key")) {
            this.certificate = ownCertificateInputStream.lines().collect(Collectors.joining("\n"));
            var privateKeyBytes = privateKeyInputStream.readAllBytes();
            this.privateKey = CryptoUtils.readPrivateKey(privateKeyBytes);
        }

        this.certificateMap = new HashMap<>();
        this.testBytesMap = new HashMap<>();
        this.secretStore = new SecureStoreInMemory();
        this.pskStore = new SecureStoreInMemory();
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

        return Base64.getEncoder().encode(this.certificate.getBytes());
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
        var thirdPartyPublicKey = CryptoUtils.decodePublicPoint(publicParams, ((BCECPublicKey)ownKeyPair.getPublic()).getParams());
        var publicKeyPoint = CryptoUtils.packagePublicEcPoint((BCECPublicKey) ownKeyPair.getPublic());
        var signatureOfPublicKey = CryptoUtils
                .signEcdsa(publicKeyPoint, privateKey);

        var testBytes = CryptoUtils.generateRandomSequence(TEST_BYTES_LENGTH);
        testBytesMap.put(clientAddress, testBytes);

        var base64Encoder = Base64.getEncoder();

        var serverPayload = new ServerPayload(
                base64Encoder.encodeToString(publicKeyPoint) ,
                base64Encoder.encodeToString(signatureOfPublicKey),
                base64Encoder.encodeToString(testBytes));

        var sharedSecret = CryptoUtils
                .generateSharedSecret(ownKeyPair.getPrivate(), thirdPartyPublicKey);
        log.info("Shared secret for {} was generated successfully", clientAddress);
        secretStore.store(clientAddress, sharedSecret);

        return serverPayload;
    }

    @Override
    public void clientFinish(String encodedPayload) throws Exception  {
        var payload = Base64.getDecoder().decode(encodedPayload);
        var clientAddress = getRequestIp();

        var iv = Arrays.copyOfRange(payload, 0, 16);
        var ciphertext = Arrays.copyOfRange(payload, 16, payload.length);
        var sessionKey = secretStore.retrieve(clientAddress);

        var secretBytes = testBytesMap.remove(clientAddress);
        var decrypted = CryptoUtils.decryptAes(ciphertext, iv, sessionKey);

        if(!Arrays.equals(secretBytes, decrypted)) {
            log.info("Payload from {} wasn't correct", clientAddress);

            throw new BadTestBytesException();
        }

        pskStore.store(clientAddress, sessionKey);
        log.info("Session with {} has been established successfully", clientAddress);
    }

    private String getRequestIp()
    {
        return ((ServletRequestAttributes) RequestContextHolder.currentRequestAttributes())
                .getRequest()
                .getRemoteAddr();
    }
}
