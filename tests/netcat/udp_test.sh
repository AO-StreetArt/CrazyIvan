#!/bin/bash

# Setup the UDP message
python register_device.py $(ip addr show eth0 | grep -Po 'inet \K[\d.]+')
# Send the UDP Message with NetCat
cat udp_message_upd.txt | nc -u $(ip addr show eth0 | grep -Po 'inet \K[\d.]+') 8764
printf "Completing UDP Test -- Bash\n"
