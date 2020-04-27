#!/bin/bash
cd $(dirname $0)
/usr/lib/jvm/java-11-openjdk-amd64/bin/java -cp target/stroke-1.0-SNAPSHOT.jar:$(cat cp.txt) br.impa.app.App $*
