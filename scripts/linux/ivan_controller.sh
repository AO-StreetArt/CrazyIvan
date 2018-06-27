#!/bin/bash

OPT="-none"

if [ "$#" -gt 0 ]; then
  OPT=$1
fi

if [ $OPT = "start" ]; then

  printf "Starting CrazyIvan\n"
  /usr/local/bin/crazy_ivan &
  $! > /var/crazyivan/pid.txt
  exit 0

else

  printf "Stopping CrazyIvan\n"
  kill $(cat pid.txt)
  NETWORK_INTERFACE_NAME=$(route | grep '^default' | grep -o '[^ ]*$')
  NETWORK_INTERFACE_ADDRESS=$(ip addr show $NETWORK_INTERFACE_NAME | grep -Po 'inet \K[\d.]+')
  IVAN_UDP_PORT=$(cat $AOSSL_PROPS_FILE | grep udp.port | grep -o [0-9]*)
  echo "kill" | nc -u $NETWORK_INTERFACE_ADDRESS $IVAN_UDP_PORT
  exit 0

fi
