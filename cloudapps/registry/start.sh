#!/bin/sh

echo 'starting registry...'
$JAVA_HOME/bin/keytool -importcert -trustcacerts -cacerts -storepass changeit -noprompt -file /usr/app/ca.crt -alias ownCA

/usr/app/wait.sh configserver:8800 --timeout=0 --strict -- java -jar /usr/app/registry-0.0.1-SNAPSHOT.jar