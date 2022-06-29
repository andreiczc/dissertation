FROM openjdk:17.0.1-jdk-slim

LABEL title="config-server" \
  description="Cloud Config Server"

ARG VERSION=0.0.1

RUN mkdir -p /usr/app
WORKDIR /usr/app
ADD target/config-server-${VERSION}-SNAPSHOT.jar .
ADD start.sh .
ADD ca.crt .

RUN apt update
RUN apt install -y curl
RUN curl https://raw.githubusercontent.com/vishnubob/wait-for-it/master/wait-for-it.sh > wait.sh
RUN chmod +x wait.sh
RUN chmod +x start.sh

EXPOSE 8800

ENTRYPOINT ["/usr/app/start.sh"]