#!/bin/bash

INTERVAL=${1:-60}
DURATION=${2:-1200}
ITER=0

while [ $ITER -lt $DURATION ]; do echo "Waiting for Docker Build";sleep $INTERVAL; ITER=$(( $ITER + $INTERVAL ));done
