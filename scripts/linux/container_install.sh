#!/bin/bash
#This script will install Crazy Ivan by cloning it from github and building from source

BRANCH="v2"

if [ "$#" -gt 0 ]; then
  BRANCH=$1
fi

apt-get -y install git
git clone --depth=50 --branch=$BRANCH https://github.com/AO-StreetArt/CrazyIvan.git
cd CrazyIvan/scripts/linux/deb && ./build_deps.sh
export LD_LIBRARY_PATH="/usr/local/lib:$LD_LIBRARY_PATH"
cd ../../.. && make && make test
