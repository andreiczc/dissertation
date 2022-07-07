package ro.sec.coap.repo;

import javax.crypto.SecretKey;
import java.net.InetAddress;

public interface SecureStore {

    void store(InetAddress key, SecretKey value);

    SecretKey retrieve(InetAddress key);
}
