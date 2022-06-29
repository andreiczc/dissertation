#!/bin/sh

echo 'starting config-server...'
$JAVA_HOME/bin/keytool -importcert -trustcacerts -cacerts -storepass changeit -noprompt -file /usr/app/ca.crt -alias ownCA

/usr/app/wait.sh mysql:3306 --timeout=0 --strict -- java -jar /usr/app/config-server-0.0.1-SNAPSHOT.jar