#!/bin/sh
mvn -f registry/pom.xml package -DskipTests
mvn -f gateway/pom.xml package -DskipTests
mvn -f dbapp/pom.xml package -DskipTests
docker build -t registry ./registry
docker build -t gateway ./gateway
docker build -t dbapp ./dbapp