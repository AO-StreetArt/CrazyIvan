#!/bin/bash
#This script will install Crazy Ivan by cloning it from github and building from source

set -e

BRANCH="v2"

if [ "$#" -gt 0 ]; then
  BRANCH=$1
fi

apt-get update
apt-get -y install software-properties-common
add-apt-repository -y ppa:ubuntu-toolchain-r/test
apt-get -y update
apt-get -y install git gcc-6 g++6 make
ln -s /usr/bin/gcc-6 /usr/bin/gcc
ln -s /usr/bin/g++-6 /usr/bin/g++
git clone --depth=50 --branch=$BRANCH https://github.com/AO-StreetArt/CrazyIvan.git
mkdir ivan_deps
cp CrazyIvan/scripts/linux/deb/build_deps.sh ivan_deps
cd ivan_deps && ./build_deps.sh
export LD_LIBRARY_PATH="/usr/local/lib:$LD_LIBRARY_PATH"
cd ../CrazyIvan && make && make test
