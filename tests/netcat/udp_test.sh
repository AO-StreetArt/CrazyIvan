#!/bin/bash

# Setup the UDP message
python register_device.py $(ip addr show eth0 | grep -Po 'inet \K[\d.]+')
printf "Sleeping"
# Sleep long enough for the cache loader to load the scene
python sleep.py 10
# Send the UDP Message with NetCat
printf "Sending UDP message"
NETWORK_INTERFACE_NAME=$(route | grep '^default' | grep -o '[^ ]*$')
NETWORK_INTERFACE_ADDRESS=$(ip addr show $NETWORK_INTERFACE_NAME | grep -Po 'inet \K[\d.]+')
printf $NETWORK_INTERFACE_NAME
printf $NETWORK_INTERFACE_ADDRESS
cat udp_message_upd.txt | nc -u $NETWORK_INTERFACE_ADDRESS 8764
