#!/bin/sh

echo 'starting container...'
$JAVA_HOME/bin/keytool -importcert -trustcacerts -cacerts -storepass changeit -noprompt -file /usr/app/ca.crt -alias ownCA

java -jar "/usr/app/dbapp-0.0.1-SNAPSHOT.jar"