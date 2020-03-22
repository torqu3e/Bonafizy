#!/bin/bash

if [[ $# -eq 0 ]] ; then
    echo "usage: $0 FILENAME"
    exit 0
fi

RESULTS_FILE="test_0.1.1.log"
KETTLE_IP="192.168.1.186"
API_ENDPOINTS="power/off hold/off hold/on power/on hold/off hold/on hold/on power/off brew brew power/off state"

date > $1

for i in $API_ENDPOINTS; do 
    curl -s http://$KETTLE_IP/$i; 
    sleep 1; 
done >> $1

diff $RESULTS_FILE $1