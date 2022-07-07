package ro.sec.coap;

import com.fasterxml.jackson.databind.ObjectMapper;
import org.bouncycastle.jcajce.provider.asymmetric.ec.BCECPublicKey;
import org.eclipse.californium.core.CoapResource;
import org.eclipse.californium.core.coap.MediaTypeRegistry;
import org.eclipse.californium.core.server.resources.CoapExchange;
import org.eclipse.californium.core.server.resources.Resource;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import ro.sec.coap.dto.ClientPayload;
import ro.sec.coap.dto.ServerPayload;
import ro.sec.coap.repo.SecureStore;
import ro.sec.coap.repo.SecureStoreInMemory;
import ro.sec.coap.web.AttestationServer;
import ro.sec.crypto.CryptoUtils;

import java.net.InetAddress;
import java.security.PrivateKey;
import java.security.cert.Certificate;
import java.security.cert.CertificateFactory;
import java.util.*;

import static org.eclipse.californium.core.coap.CoAP.ResponseCode.BAD_REQUEST;
import static org.eclipse.californium.core.coap.CoAP.ResponseCode.VALID;

// TODO add certificate validation
public class Application {

    private static final Logger log = LoggerFactory.getLogger(Application.class);
    private static final int TEST_BYTES_LENGTH = 16;
    private static final String CLIENT_HELLO_RSC = "clientHello";
    private static final String CLIENT_TEST = "test";
    private static final String KEY_EXCHANGE_RSC = "keyExchange";
    private static final String CLIENT_FINISHED_RSC = "clientFinished";
    private static final String LOG_MSG_FORMAT = "{} from {}. Payload: {}";

    public static void main(String[] args) throws Exception {
        Certificate tempOwnCertificate = null;
        PrivateKey tempPrivateKey = null;

        try (var ownCertificateInputStream = Application.class.getClassLoader().getResourceAsStream("server.crt");
                var privateKeyInputStream = Application.class.getClassLoader().getResourceAsStream("server.key")) {
            tempOwnCertificate = CertificateFactory
                    .getInstance("X.509")
                    .generateCertificate(ownCertificateInputStream);
            var privateKeyBytes = privateKeyInputStream.readAllBytes();
            tempPrivateKey = CryptoUtils.readPrivateKey(privateKeyBytes);
        }

        var ownCertificate = tempOwnCertificate;
        var privateKey = tempPrivateKey;

        var certificateMap = new HashMap<InetAddress, Certificate>();
        var testBytesMap = new HashMap<InetAddress, byte[]>();
        SecureStore secretStore = new SecureStoreInMemory();
        SecureStore pskStore = new SecureStoreInMemory();

        var resources = new ArrayList<Resource>();

        resources.add(new CoapResource(CLIENT_TEST) {
            @Override
            public void handleGET(CoapExchange exchange) {
                log.info("Received test from " + exchange.getSourceAddress());

                exchange.respond(VALID, "Hello from server");
            }
        });

        resources.add(new CoapResource(CLIENT_HELLO_RSC) {
            @Override
            public void handlePOST(CoapExchange exchange) {
                try {
                    log.info(LOG_MSG_FORMAT, CLIENT_HELLO_RSC, exchange.getSourceAddress(), CryptoUtils.toHex(exchange.getRequestPayload()));

                    var certificateDecoded = Base64.getDecoder().decode(exchange.getRequestPayload());
                    var certificate = CryptoUtils.readCertificate(certificateDecoded);

                    certificateMap.put(
                            exchange.getSourceAddress(),
                            certificate
                    );

                    exchange.respond(VALID, Base64.getEncoder().encode(ownCertificate.getEncoded()));
                } catch (Exception e) {
                    log.error(e.toString());
                }
            }
        });

        resources.add(new CoapResource(KEY_EXCHANGE_RSC) {
            @Override
            public void handlePOST(CoapExchange exchange) {
                try {
                    var clientAddress = exchange.getSourceAddress();

                    log.info(LOG_MSG_FORMAT, KEY_EXCHANGE_RSC, clientAddress, CryptoUtils.toHex(exchange.getRequestPayload()));

                    var clientJson = exchange.getRequestPayload();
                    var clientPayload = new ObjectMapper().readValue(clientJson, ClientPayload.class);
                    var thirdPartyCertificate = certificateMap.get(clientAddress);

                    var clientSignatureVerifies = CryptoUtils.verifyEcdsa(
                            clientPayload.getPublicParams(),
                            clientPayload.getSignature(),
                            thirdPartyCertificate.getPublicKey()
                    );

                    if (!clientSignatureVerifies) {
                        log.info("Signature from {} didn't verify!", clientAddress);
                        exchange.respond(BAD_REQUEST, "Signature doesn't verify... Can't continue");

                        return;
                    }

                    log.info("Signature from {} verifies", clientAddress);
                    exchange.accept();

                    var ownKeyPair = CryptoUtils.generateEcKeyPair();
                    var thirdPartyPublicKey = CryptoUtils.decodePublicPoint(clientPayload.getPublicParams(), ((BCECPublicKey)ownKeyPair.getPublic()).getParams());
                    var publicKeyPoint = CryptoUtils.packagePublicEcPoint((BCECPublicKey) ownKeyPair.getPublic());
                    var signatureOfPublicKey = CryptoUtils
                            .signEcdsa(publicKeyPoint, privateKey);

                    var testBytes = CryptoUtils.generateRandomSequence(TEST_BYTES_LENGTH);
                    testBytesMap.put(clientAddress, testBytes);

                    var serverPayload = new ServerPayload(publicKeyPoint,
                            signatureOfPublicKey,
                            testBytes);

                    var serverPayloadJson = new ObjectMapper()
                            .writeValueAsString(serverPayload);

                    var sharedSecret = CryptoUtils
                            .generateSharedSecret(ownKeyPair.getPrivate(), thirdPartyPublicKey);
                    log.info("Shared secret for {} was generated successfully", clientAddress);
                    secretStore.store(clientAddress, sharedSecret);

                    exchange.respond(VALID, serverPayloadJson, MediaTypeRegistry.APPLICATION_JSON);
                } catch (Exception e) {
                    log.error(e.toString());
                }
            }
        });

        resources.add(new CoapResource(CLIENT_FINISHED_RSC) {
            @Override
            public void handlePOST(CoapExchange exchange) {
                try {
                    var clientAddress = exchange.getSourceAddress();

                    log.info(LOG_MSG_FORMAT, KEY_EXCHANGE_RSC, clientAddress, CryptoUtils.toHex(exchange.getRequestPayload()));

                    var payload = exchange.getRequestPayload();
                    var iv = Arrays.copyOfRange(payload, 0, 16);
                    var ciphertext = Arrays.copyOfRange(payload, 16, payload.length);
                    var sessionKey = secretStore.retrieve(clientAddress);

                    var secretBytes = testBytesMap.get(clientAddress);
                    var decrypted = CryptoUtils.decryptAes(ciphertext, iv, sessionKey);

                    if(!Arrays.equals(secretBytes, decrypted)) {
                        log.info("Payload from {} wasn't correct", clientAddress);
                        exchange.respond(BAD_REQUEST, "Test wasn't correct. Try again!");

                        return;
                    }

                    pskStore.store(clientAddress, sessionKey);
                    log.info("Session with {} has been established successfully", clientAddress);

                    exchange.respond(VALID);
                } catch (Exception e) {
                    log.error(e.toString());
                }
            }
        });

        startServer(resources);
    }

    private static void startServer(List<Resource> resources) {
        try (var server = new AttestationServer(resources)) {
            while (true) {
                Thread.sleep(5000);
            }
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }
}
