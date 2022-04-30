package ro.sec.attestation.repo;

import javax.crypto.SecretKey;

public interface SecureStore {

    void store(String key, SecretKey value);

    SecretKey retrieve(String key);
}
