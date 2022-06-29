#!/bin/sh

echo 'starting dbapp...'
$JAVA_HOME/bin/keytool -importcert -trustcacerts -cacerts -storepass changeit -noprompt -file /usr/app/ca.crt -alias ownCA

/usr/app/wait.sh registry:8761 --timeout=0 --strict -- java -Dvault.token=$WRAPPING_TOKEN -jar "/usr/app/dbapp-0.0.1-SNAPSHOT.jar"