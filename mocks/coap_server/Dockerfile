FROM openjdk:17-slim

LABEL title = "CoAP-Server"

ARG VERSION=1.0

RUN mkdir -p /usr/app
WORKDIR /usr/app
ADD target/coap_server-${VERSION}-SNAPSHOT.jar app.jar

EXPOSE 5683

ENTRYPOINT ["java", "-jar", "/usr/app/app.jar"]