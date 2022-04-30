package ro.sec.attestation.repo;

import org.springframework.stereotype.Component;

import javax.crypto.SecretKey;
import java.util.HashMap;
import java.util.Map;

public class SecureStoreInMemory implements SecureStore {

    private final Map<String, SecretKey> store;

    public SecureStoreInMemory() {
        this.store = new HashMap<>();
    }

    @Override
    public void store(String key, SecretKey value) {
        if (key == null || value == null) {
            throw new RuntimeException(
                    String.format("One of the values was null! key: %b, value: %b", key == null, value == null)
            );
        }

        store.put(key, value);
    }

    @Override
    public SecretKey retrieve(String key) {
        if (key == null) {
            throw new RuntimeException("Input parameter key was null!");
        }

        return store.remove(key);
    }
}
