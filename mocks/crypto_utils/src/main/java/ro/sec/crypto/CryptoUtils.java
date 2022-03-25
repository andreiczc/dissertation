package ro.sec.crypto;

import org.bouncycastle.jce.ECNamedCurveTable;

import javax.crypto.*;
import javax.crypto.spec.IvParameterSpec;
import java.io.ByteArrayInputStream;
import java.nio.file.Files;
import java.nio.file.Path;
import java.security.*;
import java.security.cert.Certificate;
import java.security.cert.CertificateException;
import java.security.cert.CertificateFactory;
import java.security.cert.X509Certificate;
import java.security.spec.ECParameterSpec;
import java.security.spec.PKCS8EncodedKeySpec;
import java.security.spec.X509EncodedKeySpec;
import java.util.Base64;

public class CryptoUtils {

    static {
        loadBcProvider();
    }

    private static final String BOUNCY_CASTLE_PROVIDER = "BC";

    public static byte[] generateRandomSequence(int length) {
        if (length <= 0) {
            throw new RuntimeException("Bad value for input parameter length");
        }

        var result = new byte[length];
        var secureRandom = new SecureRandom();
        secureRandom.nextBytes(result);

        return result;
    }

    public static byte[] encryptAes(byte[] plaintext, byte[] iv, SecretKey secretKey) throws NoSuchPaddingException, NoSuchAlgorithmException, InvalidKeyException, IllegalBlockSizeException, BadPaddingException, InvalidAlgorithmParameterException {
        if (plaintext == null || secretKey == null || iv == null) {
            throw new RuntimeException(
                    String.format("One of the args was null. Plaintext: %b, IV: %b, SecretKey: %b",
                            plaintext == null, iv == null, secretKey == null));
        }

        var cipher = Cipher.getInstance("AES/CBC/PKCS5Padding");
        cipher.init(Cipher.ENCRYPT_MODE,
                secretKey,
                new IvParameterSpec(iv));

        return cipher.doFinal(plaintext);
    }

    public static byte[] decryptAes(byte[] cipherText, byte[] iv, SecretKey secretKey) throws NoSuchPaddingException, NoSuchAlgorithmException, InvalidKeyException, IllegalBlockSizeException, BadPaddingException, InvalidAlgorithmParameterException {
        if (cipherText == null || secretKey == null || iv == null) {
            throw new RuntimeException(
                    String.format("One of the args was null. Ciphertext: %b, IV: %b, SecretKey: %b",
                            cipherText == null, iv == null, secretKey == null));
        }

        var cipher = Cipher.getInstance("AES/CBC/PKCS5Padding");
        cipher.init(Cipher.DECRYPT_MODE, secretKey, new IvParameterSpec(iv));

        return cipher.doFinal(cipherText);
    }

    public static String toHex(byte[] input) {
        var formatter = new StringBuilder();
        for (var piece : input) {
            formatter.append(String.format("%02x ", piece));
        }

        return formatter.toString();
    }

    public static byte[] toByteArray(String input) {
        var tokens = input.split(" ");
        var result = new byte[tokens.length];

        for (var i = 0; i < tokens.length; ++i) {
            result[i] = (byte) Integer.parseInt(tokens[i], 16);
        }

        return result;
    }

    public static byte[] signEcdsa(byte[] content, PrivateKey privateKey) throws NoSuchAlgorithmException, NoSuchProviderException, InvalidKeyException, SignatureException {
        var signature = Signature.getInstance("SHA384withECDSA", BOUNCY_CASTLE_PROVIDER);
        signature.initSign(privateKey);
        signature.update(content);

        return signature.sign();
    }

    public static boolean verifyEcdsa(byte[] content, byte[] digitalSignature, PublicKey publicKey) throws NoSuchAlgorithmException, NoSuchProviderException, InvalidKeyException, SignatureException {
        var signature = Signature.getInstance("SHA384withECDSA", BOUNCY_CASTLE_PROVIDER);
        signature.initVerify(publicKey);
        signature.update(content);

        return signature.verify(digitalSignature);
    }

    public static PrivateKey readPrivateKey(Path path) throws Exception {
        var content = Files.readAllBytes(path);
        content = new String(content)
                .replace("-----BEGIN PRIVATE KEY-----", "")
                .replaceAll("\n", "")
                .replace("-----END PRIVATE KEY-----", "")
                .getBytes();
        content = Base64.getDecoder().decode(content);

        var privateKey = new PKCS8EncodedKeySpec(content);

        return KeyFactory.getInstance("ECDSA", BOUNCY_CASTLE_PROVIDER).generatePrivate(privateKey);
    }

    public static PublicKey readPublicKey(byte[] content) throws Exception {
        var keyFactory = KeyFactory.getInstance("ECDH", BOUNCY_CASTLE_PROVIDER);
        var keySpec = new X509EncodedKeySpec(content);

        return keyFactory.generatePublic(keySpec);
    }

    public static Certificate readCertificate(byte[] certificate) throws CertificateException {
        var certificateFactory = CertificateFactory.getInstance("X.509");

        return certificateFactory.generateCertificate(new ByteArrayInputStream(certificate));
    }

    public static boolean verifyCertificate(X509Certificate certificate, PublicKey rootCaPublicKey) {
        try {
            certificate.verify(rootCaPublicKey);
            return true;
        } catch (Exception e) {
            return false;
        }
    }

    public static KeyPair generateEcKeyPair() throws NoSuchAlgorithmException, NoSuchProviderException, InvalidAlgorithmParameterException {
        var keyPairGenerator = KeyPairGenerator.getInstance("ECDH", BOUNCY_CASTLE_PROVIDER);
        keyPairGenerator.initialize(ECNamedCurveTable.getParameterSpec("brainpoolp256r1"));

        return keyPairGenerator.generateKeyPair();
    }

    public static KeyPair generateEcKeyPair(ECParameterSpec spec) throws NoSuchAlgorithmException, NoSuchProviderException, InvalidAlgorithmParameterException {
        var keyPairGenerator = KeyPairGenerator.getInstance("ECDH", BOUNCY_CASTLE_PROVIDER);
        keyPairGenerator.initialize(spec);

        return keyPairGenerator.generateKeyPair();
    }

    public static SecretKey generateSharedSecret(PrivateKey own, PublicKey thirdParty) throws NoSuchAlgorithmException, NoSuchProviderException, InvalidKeyException {
        var keyAgreement = KeyAgreement.getInstance("ECDH", BOUNCY_CASTLE_PROVIDER);
        keyAgreement.init(own);
        keyAgreement.doPhase(thirdParty, true);

        return keyAgreement.generateSecret("AES");
    }

    private static void loadBcProvider() {
        var provider = Security.getProvider(BOUNCY_CASTLE_PROVIDER);
        if (provider == null) {
            Security.addProvider(new org.bouncycastle.jce.provider.BouncyCastleProvider());

            provider = Security.getProvider(BOUNCY_CASTLE_PROVIDER);
            if (provider == null) {
                throw new RuntimeException("Can't load BC Provider");
            }
        }
    }
}
