FROM openjdk:17.0.1-jdk-slim

LABEL title="registry" \
  description="Eureka Registry"

ARG VERSION=0.0.1

RUN mkdir -p /usr/app
WORKDIR /usr/app
ADD target/registry-${VERSION}-SNAPSHOT.jar .

EXPOSE 8761

ENTRYPOINT ["/usr/app/start.sh"]