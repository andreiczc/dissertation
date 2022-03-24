package ro.sec.coap;

import org.eclipse.californium.core.CoapClient;
import org.eclipse.californium.core.coap.MediaTypeRegistry;
import org.eclipse.californium.core.config.CoapConfig;
import org.eclipse.californium.elements.config.UdpConfig;
import ro.sec.crypto.CryptoUtils;

import java.io.FileInputStream;
import java.nio.file.Path;
import java.security.cert.CertificateFactory;

public class Application {

    static {
        CoapConfig.register();
        UdpConfig.register();
    }

    public static void main(String[] args) throws Exception {
        var certificateFactory = CertificateFactory.getInstance("X.509");
        var certificate = certificateFactory.generateCertificate(new FileInputStream("src/main/resources/server.crt"));
        var privateKey = CryptoUtils.readPrivateKey(Path.of("src/main/resources/server.key"));

        var client = new CoapClient("coap://localhost:5683/clientHello");
        client.useNONs();

        var serializedCertificate = CryptoUtils.toHex(certificate.getEncoded());

        var ownKeyPair = CryptoUtils.generateEcKeyPair();
        var serializedPublicKey = CryptoUtils.toHex(ownKeyPair.getPublic().getEncoded());

        var signatureOfPublicKey = CryptoUtils.signEcdsa(ownKeyPair.getPublic().getEncoded(), privateKey);
        var serializedSignature = CryptoUtils.toHex(signatureOfPublicKey);

        var payload = """
                {
                    "certificate": %s,
                    "public": %s,
                    "signature": %s
                }
                """.formatted(serializedCertificate, serializedPublicKey, serializedSignature);

        var response = client.post(payload, MediaTypeRegistry.APPLICATION_JSON);
    }
}
