package ro.sec.coap.repo;

import javax.crypto.SecretKey;
import java.net.InetAddress;
import java.util.HashMap;
import java.util.Map;

public class SecureStoreInMemory implements SecureStore {

    private final Map<InetAddress, SecretKey> store;

    public SecureStoreInMemory() {
        this.store = new HashMap<>();
    }

    @Override
    public void store(InetAddress key, SecretKey value) {
        if (key == null || value == null) {
            throw new RuntimeException(
                    String.format("One of the values was null! key: %b, value: %b", key == null, value == null)
            );
        }

        store.put(key, value);
    }

    @Override
    public SecretKey retrieve(InetAddress key) {
        if (key == null) {
            throw new RuntimeException("Input parameter key was null!");
        }

        return store.remove(key);
    }
}
