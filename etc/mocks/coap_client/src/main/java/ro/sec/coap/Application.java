package ro.sec.coap;

import com.fasterxml.jackson.databind.ObjectMapper;
import org.bouncycastle.asn1.x9.ECNamedCurveTable;
import org.bouncycastle.jcajce.provider.asymmetric.ec.BCECPublicKey;
import org.bouncycastle.jce.ECPointUtil;
import org.bouncycastle.jce.spec.ECNamedCurveSpec;
import org.bouncycastle.util.Arrays;
import org.eclipse.californium.core.CoapClient;
import org.eclipse.californium.core.coap.MediaTypeRegistry;
import org.eclipse.californium.core.config.CoapConfig;
import org.eclipse.californium.elements.config.UdpConfig;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import ro.sec.crypto.CryptoUtils;

import java.io.FileInputStream;
import java.nio.file.Path;
import java.security.cert.CertificateFactory;
import java.security.spec.EllipticCurve;
import java.util.Base64;

import static org.eclipse.californium.core.coap.CoAP.ResponseCode.VALID;

public class Application {

    static {
        CoapConfig.register();
        UdpConfig.register();
    }

    private static final Logger log = LoggerFactory.getLogger(Application.class);
    private static final String SERVER_URL = "coap://localhost:5683/";

    public static void main(String[] args) throws Exception {
        var certificateFactory = CertificateFactory.getInstance("X.509");
        var privateKey = CryptoUtils.readPrivateKey(Path.of("src/main/resources/client.key"));
        var certificate = certificateFactory.generateCertificate(new FileInputStream("src/main/resources/client.crt"));

        var client = new CoapClient( SERVER_URL + "clientHello");
        var serverCertResponse = client.post(Base64.getEncoder().encode(certificate.getEncoded()), MediaTypeRegistry.APPLICATION_OCTET_STREAM);

        var decodedCert = Base64.getDecoder().decode(serverCertResponse.getPayload());
        var serverCert = CryptoUtils.readCertificate(decodedCert);
        var ownKeyPair = CryptoUtils.generateEcKeyPair();
        var publicPoint = CryptoUtils.packagePublicEcPoint((BCECPublicKey) ownKeyPair.getPublic());
        var signatureOfPublicKey = CryptoUtils.signEcdsa(publicPoint, privateKey);

        var payloadObj = new ClientPayload(publicPoint, signatureOfPublicKey);
        var payload = new ObjectMapper()
                .writeValueAsString(payloadObj);

        client.setURI(SERVER_URL + "keyExchange");
        var response = client.post(payload, MediaTypeRegistry.APPLICATION_JSON);

        var serverPayload = new ObjectMapper()
                .readValue(response.getPayload(), ServerPayload.class);

        var signatureVerifies = CryptoUtils.verifyEcdsa(
                serverPayload.getPublicParams(),
                serverPayload.getSignature(),
                serverCert.getPublicKey()
        );

        if(!signatureVerifies) {
            log.error("Signature doesn't verify... Can't continue");
            return;
        }

        log.info("Server signature verifies");
        var serverPubKey = CryptoUtils.decodePublicPoint(serverPayload.getPublicParams(), ((BCECPublicKey) ownKeyPair.getPublic()).getParams());
        var sharedSecret = CryptoUtils.generateSharedSecret(
                ownKeyPair.getPrivate(),
                serverPubKey
        );

        var plaintext = serverPayload.getTest();
        var iv = CryptoUtils.generateRandomSequence(16);
        var ciphertext = CryptoUtils.encryptAes(plaintext, iv, sharedSecret);

        var finishedPayload = Arrays.concatenate(iv, ciphertext);

        client.setURI(SERVER_URL + "clientFinished");
        var responseFinished = client.post(finishedPayload, MediaTypeRegistry.APPLICATION_OCTET_STREAM);

        if(responseFinished.getCode() != VALID) {
            log.error("Something went wrong");
        }

        log.info("Session now established!");
    }
}
