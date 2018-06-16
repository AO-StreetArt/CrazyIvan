#!/bin/bash

# Setup the UDP message
python register_device.py $(ip addr show enp2s0 | grep -Po 'inet \K[\d.]+')
# Sleep long enough for the cache loader to load the scene
sleep 10
# Send the UDP Message with NetCat
cat udp_message_upd.txt | nc -u $(ip addr show enp2s0 | grep -Po 'inet \K[\d.]+') 8764
printf "Completing UDP Test -- Bash\n"
