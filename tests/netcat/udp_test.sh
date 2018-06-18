#!/bin/bash

# Setup the UDP message
python register_device.py $(ip addr show eth0 | grep -Po 'inet \K[\d.]+')
printf "Sleeping"
# Sleep long enough for the cache loader to load the scene
python sleep.py 10
# Send the UDP Message with NetCat
printf "Sending UDP message"
cat udp_message_upd.txt | nc -u $(ip addr show eth0 | grep -Po 'inet \K[\d.]+') 8764
printf "Completing UDP Test -- Bash\n"
# Sleep long enough to let messages send
python sleep.py 5
# Remove the scene from the cache
python remove_cached_scene.py $(cat scene_key.txt)
