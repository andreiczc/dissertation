import org.bouncycastle.jce.ECNamedCurveTable;

import javax.crypto.KeyAgreement;
import javax.crypto.SecretKey;
import java.security.*;
import java.util.Arrays;

public class Application {

    static {
        loadBcProvider();
    }

    private final static String BOUNCY_CASTLE_PROVIDER = "BC";

    public static void main(String[] args) throws NoSuchAlgorithmException, NoSuchProviderException, InvalidAlgorithmParameterException, InvalidKeyException {
        // Generate first Alice key pair
        var aliceKeyPair = generateEcKeyPair();
        var bobKeyPair = generateEcKeyPair();

        var aliceSecret = generateSharedSecret(aliceKeyPair.getPrivate(), bobKeyPair.getPublic());
        var bobSecret = generateSharedSecret(bobKeyPair.getPrivate(), aliceKeyPair.getPublic());

        System.out.println(Arrays.equals(aliceSecret.getEncoded(), bobSecret.getEncoded()));
    }

    private static KeyPair generateEcKeyPair() throws NoSuchAlgorithmException, NoSuchProviderException, InvalidAlgorithmParameterException {
        var keyPairGenerator = KeyPairGenerator.getInstance("ECDH", BOUNCY_CASTLE_PROVIDER);
        keyPairGenerator.initialize(ECNamedCurveTable.getParameterSpec("brainpoolp256r1"));

        return keyPairGenerator.generateKeyPair();
    }

    private static SecretKey generateSharedSecret(PrivateKey own, PublicKey thirdParty) throws NoSuchAlgorithmException, NoSuchProviderException, InvalidKeyException {
        var keyAgreement = KeyAgreement.getInstance("ECDH", BOUNCY_CASTLE_PROVIDER);
        keyAgreement.init(own);
        keyAgreement.doPhase(thirdParty, true);

        return keyAgreement.generateSecret("AES");
    }

    private static void loadBcProvider() {
        var provider = Security.getProvider(BOUNCY_CASTLE_PROVIDER);
        if(provider == null) {
            Security.addProvider(new org.bouncycastle.jce.provider.BouncyCastleProvider());

            provider = Security.getProvider(BOUNCY_CASTLE_PROVIDER);
            if(provider == null) {
                throw new RuntimeException("Can't load BC Provider");
            }
        }

        System.out.println("BC loaded now");
    }
}
