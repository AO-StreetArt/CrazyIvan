#!/bin/bash
set -e
#This script will attempt to build CrazyIvan dependencies

#Based on Ubuntu 14.04 LTS
#Not intended for use with other OS (should function correctly with Debian 7, untested)

COMPILER="g++"
INSTALL_OPT="-none"
if [ "$#" -gt 0 ]; then
  COMPILER=$1
fi
if [ "$#" -gt 1 ]; then
  INSTALL_OPT=$2
fi

printf "Creating Dependency Folder"
PRE=./downloads
RETURN=..
mkdir $PRE

printf "Calling apt-get update"

#Update the Ubuntu Server
apt-get -y update
apt-get install -y git libboost-all-dev openssl libssl-dev wget

# Build and install NeoCpp
if [ ! -d /usr/local/include/neocpp ]; then

  git clone https://github.com/AO-StreetArt/NeoCpp.git
  #Build the dependencies for the shared service library
  mkdir $PRE/neocpp_deps
  cp NeoCpp/scripts/linux/deb/build_deps.sh $PRE/neocpp_deps/
  cd $PRE/neocpp_deps && ./build_deps.sh
  cd ../$RETURN
  cd NeoCpp && make install
  cd ../

fi

#Build & Install the Shared Service Library
if [ ! -d /usr/local/include/aossl ]; then

  wget https://github.com/AO-StreetArt/AOSharedServiceLibrary/releases/download/v2.4.4/aossl-deb-2.4.4.tar.gz
  tar -xvzf aossl-deb-2.4.4.tar.gz

  #Build the dependencies for the shared service library
  mkdir $PRE/aossl_deps
  cp aossl-deb/deps/build_deps.sh $PRE/aossl_deps/
  cd $PRE/aossl_deps && ./build_deps.sh $INSTALL_OPT
  cd ../$RETURN

  #Build the shared service library
  cd aossl-deb && make clean && make CC=$COMPILER && make install
  cd ../

fi

# Install GLM
if [ ! -d /usr/local/include/glm ]; then

  wget https://github.com/g-truc/glm/releases/download/0.9.9.0/glm-0.9.9.0.zip
  unzip glm-0.9.9.0.zip
  mkdir /usr/local/include/glm/
  cp -r glm/glm/* /usr/local/include/glm/

fi

printf "Finished installing dependencies"
