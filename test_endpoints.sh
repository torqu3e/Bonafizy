#!/bin/bash

if [[ $# -eq 0 ]] ; then
    echo "usage: $0 FILENAME"
    exit 0
fi

RESULTS_FILE="test_0.1.1.log"
KETTLE_HOSTNAME="kettle.local"
KETTLE_IP=$(ping -c 1 $KETTLE_HOSTNAME | grep icmp_seq | awk '{print $4}' | tr -d ':')
API_ENDPOINTS="power/off hold/off hold/on power/on hold/off hold/on hold/on power/off brew brew power/off state"
start_time=$(date +%s)

date > $1
echo "Kettle IP: $KETTLE_IP" >> $1

for i in $API_ENDPOINTS; do 
    echo "curl -s http://$KETTLE_IP/$i";
    curl -s http://$KETTLE_IP/$i | tee -a $1; 
    sleep 1; 
done

end_time=$(date +%s)
echo "Test time: $((end_time - start_time)) seconds" | tee -a $1
diff $RESULTS_FILE $1
