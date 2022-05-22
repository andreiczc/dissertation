#!/bin/sh
mvn -f registry/pom.xml package -DskipTests
mvn -f gateway/pom.xml package -DskipTests
mvn -f dbapp/pom.xml package -DskipTests
docker build -t registry_andrei ./registry
docker build -t gateway_andrei ./gateway
docker build -t dbapp_andrei ./dbapp