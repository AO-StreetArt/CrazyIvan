#!/bin/bash
set -e
#This script will attempt to build CrazyIvan dependencies

#Based on Ubuntu 14.04 LTS
#Not intended for use with other OS (should function correctly with Debian 7, untested)

printf "Creating Dependency Folder"
PRE=./downloads
RETURN=..
mkdir $PRE

printf "Calling apt-get update"

#Update the Ubuntu Server
apt-get -y update
apt-get install -y git libboost-all-dev openssl libssl-dev

#Build & Install the Shared Service Library
if [ ! -d /usr/local/include/aossl ]; then

  wget https://github.com/AO-StreetArt/AOSharedServiceLibrary/releases/download/v2.0.0/aossl-deb-2.0.0.tar.gz
  tar -xvzf aossl-deb-2.0.0.tar.gz

  #Build the dependencies for the shared service library
  mkdir $PRE/aossl_deps
  cp aossl-deb/deps/build_deps.sh $PRE/aossl_deps/
  cd $PRE/aossl_deps && sudo ./build_deps.sh
  cd ../$RETURN

  #Build the shared service library
  cd aossl-deb && make && sudo make install
  cd ../

fi

# Build and install NeoCpp
if [ ! -d /usr/local/include/neocpp ]; then

  git clone https://github.com/AO-StreetArt/NeoCpp.git
  #Build the dependencies for the shared service library
  mkdir $PRE/neocpp_deps
  cp NeoCpp/scripts/linux/deb/build_deps.sh $PRE/neocpp_deps/
  cd $PRE/neocpp_deps && sudo ./build_deps.sh
  cd ../$RETURN
  cd NeoCpp && sudo make install
  cd ../

fi

#Build POCO
wget https://pocoproject.org/releases/poco-1.9.0/poco-1.9.0-all.tar.gz
tar -xvzf poco-1.9.0-all.tar.gz
cd poco-1.9.0-all && ./configure --omit=Data/ODBC,Data/MySQL && make -s && sudo make -s install
cd ../

sudo ldconfig

printf "Finished installing dependencies"
