#!/bin/sh

echo 'starting registry...'
$JAVA_HOME/bin/keytool -importcert -trustcacerts -cacerts -storepass changeit -noprompt -file /usr/app/ca.crt -alias ownCA

java -jar /usr/app/registry-0.0.1-SNAPSHOT.jar