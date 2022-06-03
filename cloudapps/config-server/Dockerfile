FROM openjdk:17.0.1-jdk-slim

LABEL title="config-server" \
  description="Cloud Config Server"

ARG VERSION=0.0.1

RUN mkdir -p /usr/app
WORKDIR /usr/app
ADD target/config-server-${VERSION}-SNAPSHOT.jar .

RUN apt update
RUN apt install -y curl
RUN curl https://raw.githubusercontent.com/vishnubob/wait-for-it/master/wait-for-it.sh > wait.sh
RUN chmod +x wait.sh

EXPOSE 8800

ENTRYPOINT ["/usr/app/wait.sh","mysql:3306","--timeout=0","--strict", "--", "java", "-jar", "/usr/app/config-server-0.0.1-SNAPSHOT.jar"]