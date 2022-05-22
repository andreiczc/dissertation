FROM openjdk:17.0.1-jdk-slim

LABEL title="attestation-server" \
  description="Attestation Server"

ARG VERSION=0.0.1

RUN mkdir -p /usr/app
WORKDIR /usr/app
ADD target/attestation_server-${VERSION}-SNAPSHOT.jar .
ADD ca.crt .
ADD start.sh .

RUN apt update
RUN apt install -y curl
RUN curl https://raw.githubusercontent.com/vishnubob/wait-for-it/master/wait-for-it.sh > wait.sh
RUN chmod +x wait.sh
RUN chmod +x start.sh

EXPOSE 8082

ENTRYPOINT ["/usr/app/start.sh"]