package ro.sec.coap.web;

import org.eclipse.californium.core.CoapServer;
import org.eclipse.californium.core.config.CoapConfig;
import org.eclipse.californium.core.server.resources.Resource;
import org.eclipse.californium.elements.config.UdpConfig;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.util.List;

public class AttestationServer extends CoapServer implements AutoCloseable {

    private static final Logger log = LoggerFactory.getLogger(AttestationServer.class);

    static {
        CoapConfig.register();
        UdpConfig.register();
    }

    public AttestationServer(List<Resource> resourceList) {
        resourceList.forEach(this::add);
        this.start();
    }


    @Override
    public void close() {
        this.stop();
    }
}
