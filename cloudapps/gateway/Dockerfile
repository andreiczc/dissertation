FROM openjdk:17.0.1-jdk-slim

LABEL title="gateway" \
  description="Cloud Gateway"

ARG VERSION=0.0.1

RUN mkdir -p /usr/app
WORKDIR /usr/app
ADD target/gateway-${VERSION}-SNAPSHOT.jar .

RUN apt update
RUN apt install -y curl
RUN curl https://raw.githubusercontent.com/vishnubob/wait-for-it/master/wait-for-it.sh > wait.sh
RUN chmod +x wait.sh

EXPOSE 8080

ENTRYPOINT ["/usr/app/wait.sh","registry:8761","--timeout=0","--strict", "--", "java", "-jar", "/usr/app/gateway-0.0.1-SNAPSHOT.jar"]