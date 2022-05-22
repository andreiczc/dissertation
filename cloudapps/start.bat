@ECHO OFF
call mvn -f registry\pom.xml package -DskipTests
call mvn -f gateway\pom.xml package -DskipTests
call mvn -f dbapp\pom.xml package -DskipTests
call docker build -t registry .\registry
call docker build -t gateway .\gateway
call docker build -t dbapp .\dbapp
call docker-compose up -d