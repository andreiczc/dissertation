#!/bin/sh

echo 'starting container...'
$JAVA_HOME/bin/keytool -importcert -trustcacerts -cacerts -storepass changeit -noprompt -file /usr/app/ca.crt -alias ownCA

java -agentlib:jdwp=transport=dt_socket,server=y,suspend=n,address=*:5005 -jar "/usr/app/attestation_server-0.0.1-SNAPSHOT.jar"