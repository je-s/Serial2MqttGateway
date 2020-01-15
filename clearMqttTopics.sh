#!/bin/bash
HOST=$1
TOPIC=$2
QOS=$3

printf "Clearing topic(s) $TOPIC with QoS $QOS on host $HOST "

mosquitto_sub -h $HOST -t $TOPIC -q $QOS -v | while read -t 5 line; do
    topic=$(echo $line | awk '{print $1}')
    payload=$(echo $line | awk '{print $2}')

    if [ "$payload" == "(null)" ]; then
        printf "\nDone.\n"
        kill -SIGTERM $$
        break
    fi

    mosquitto_pub -h $1 -t $topic -r -n -q $QOS &
    printf "."
done
