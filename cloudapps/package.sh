#!/bin/sh
mvn -f config-server/pom.xml package -DskipTests
mvn -f registry/pom.xml package -DskipTests
mvn -f gateway/pom.xml package -DskipTests
mvn -f dbapp/pom.xml package -DskipTests
mvn -f mqtt-client/pom.xml package -DskipTests
docker build -t config-server ./config-server
docker build -t registry ./registry
docker build -t gateway ./gateway
docker build -t dbapp ./dbapp
docker build -t mqtt-client ./mqtt-client