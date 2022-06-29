#!/bin/sh

echo 'starting gateway...'
$JAVA_HOME/bin/keytool -importcert -trustcacerts -cacerts -storepass changeit -noprompt -file /usr/app/ca.crt -alias ownCA

/usr/app/wait.sh registry:8761 --timeout=0 --strict -- java -jar /usr/app/gateway-0.0.1-SNAPSHOT.jar