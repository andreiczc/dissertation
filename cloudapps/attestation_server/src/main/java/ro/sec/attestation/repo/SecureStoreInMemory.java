package ro.sec.attestation.repo;

import lombok.SneakyThrows;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.stereotype.Component;

import javax.crypto.SecretKey;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.StandardOpenOption;
import java.util.Base64;
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

@Component
public class SecureStoreInMemory implements SecureStore {

    private static final Logger log = LoggerFactory.getLogger(SecureStoreInMemory.class);

    @Value("${mqtt.psk_file}")
    private String pskFileLocation;
    private final Map<String, SecretKey> store;
    private final ExecutorService executorService;

    public SecureStoreInMemory() {
        this.store = new HashMap<>();
        executorService = Executors.newFixedThreadPool(1);
    }

    @Override
    public void store(String key, SecretKey value) {
        if (key == null || value == null) {
            throw new RuntimeException(
                    String.format("One of the values was null! key: %s, value: %s",
                            key == null ? "Null" : "Not Null",
                            value == null ? "Null" : "Not Null")
            );
        }

        store.put(key, value);
        log.info("Password was stored");

        executorService.execute(() -> {
            try {
                Files.write(Path.of(pskFileLocation), "".getBytes(), StandardOpenOption.TRUNCATE_EXISTING);
                store.forEach(this::dumpToStorage);
                restartMqtt();
            } catch (IOException e) {
                log.error("Couldn't write to file!");
            }
        });
    }

    @Override
    public SecretKey retrieve(String key) {
        if (key == null) {
            throw new RuntimeException("Input parameter key was null!");
        }

        return store.remove(key);
    }

    private void restartMqtt() {
        var processBuilder = new ProcessBuilder();
        processBuilder.command("bash", "-c", "/etc/mosquitto/restart.sh");
        try {
            processBuilder.start();
        } catch (Exception e) {
            log.error("Process had error. " + e.getMessage());
        }
    }

    @SneakyThrows
    private void dumpToStorage(String key, SecretKey value) {
        var base64Encoder = Base64.getEncoder();
        var hint = "node1";
        var secret = asciiToHex(value.getEncoded());

        var result = String.format("%s:%s\n", hint, secret);
        Files.write(
                Path.of(pskFileLocation),
                result.getBytes(),
                StandardOpenOption.APPEND
        );
    }

    private static String asciiToHex(byte[] input) {
        StringBuilder hex = new StringBuilder();
        for (var b : input) {
            hex.append(String.format("%02X", b));
        }

        return hex.toString();
    }
}
