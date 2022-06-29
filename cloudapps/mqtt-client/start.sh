#!/bin/sh

echo 'starting mqtt client...'
$JAVA_HOME/bin/keytool -importcert -trustcacerts -cacerts -storepass changeit -noprompt -file /usr/app/ca.crt -alias ownCA

java -Dvault.token=$WRAPPING_TOKEN -jar "/usr/app/mqtt-client-0.0.1-SNAPSHOT.jar"