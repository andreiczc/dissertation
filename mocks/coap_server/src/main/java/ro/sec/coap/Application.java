package ro.sec.coap;

import org.eclipse.californium.core.CoapResource;
import org.eclipse.californium.core.server.resources.CoapExchange;
import org.eclipse.californium.core.server.resources.Resource;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.util.ArrayList;
import java.util.List;

import static org.eclipse.californium.core.coap.CoAP.ResponseCode.VALID;

public class Application {

    private static final Logger log = LoggerFactory.getLogger(Application.class);

    public static void main(String[] args) {
        var resources = new ArrayList<Resource>();

        resources.add(new CoapResource("clientHello") {
            @Override
            public void handlePOST(CoapExchange exchange) {
                try {
                    var payload = exchange.getRequestPayload();
                    log.info("Payload received: {}", new String(payload));



                    exchange.respond(VALID);
                } catch (Exception e) {
                    e.printStackTrace();
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
