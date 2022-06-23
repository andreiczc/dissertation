@ECHO OFF
call mvn -f registry\pom.xml package -DskipTests
call mvn -f gateway\pom.xml package -DskipTests
call mvn -f dbapp\pom.xml package -DskipTests
call mvn -f config-server\pom.xml package -DskipTests
call mvn -f mqtt-client\pom.xml package -DskipTests
call docker build -t registry .\registry
call docker build -t gateway .\gateway
call docker build -t dbapp .\dbapp
call docker build -t config-server .\config-server
call docker build -t mqtt-client .\mqtt-client
call docker-compose up -d