################################################################

# Dockerfile to build Crazy Ivan Container Images
# Based on Ubuntu-ssh

################################################################

#Based on Ubuntu 14.04
FROM ubuntu:14.04

#Set the Maintainer
MAINTAINER Alex Barry

#Set up front end
ENV DEBIAN_FRONTEND noninteractive

#Ensure that base level build requirements are satisfied
RUN apt-get update
RUN apt-get install -y software-properties-common
RUN add-apt-repository -y ppa:ubuntu-toolchain-r/test
RUN apt-get update
RUN	apt-get install -y apt-utils debconf-utils iputils-ping wget curl mc htop ssh g++-5 build-essential libprotobuf-dev protobuf-compiler libtool pkg-config autoconf automake uuid-dev libhiredis-dev libcurl4-openssl-dev libevent-dev git liblog4cpp5-dev libkrb5-dev
RUN	apt-get clean

#Build the dependencies and place them in the correct places
RUN apt-get update

#Set up g++ 5 as the default c++ compiler
RUN unlink /usr/bin/gcc && ln -s /usr/bin/gcc-5 /usr/bin/gcc
RUN unlink /usr/bin/g++ && ln -s /usr/bin/g++-5 /usr/bin/g++

RUN gcc --version

#Get the Redis Dependencies
RUN git clone https://github.com/redis/hiredis.git ./hiredis
RUN cd ./hiredis && make && make install

#Get the Mongo Dependencies
RUN wget https://github.com/mongodb/mongo-c-driver/releases/download/1.6.0/mongo-c-driver-1.6.0.tar.gz
RUN tar xzf mongo-c-driver-1.6.0.tar.gz
RUN cd mongo-c-driver-1.6.0 && ./configure --disable-automatic-init-and-cleanup && make && sudo make install

#Get the Neo4j Dependencies
RUN mkdir $PRE/neo
RUN wget https://github.com/cleishm/libneo4j-client/releases/download/v1.2.1/libneo4j-client-1.2.1.tar.gz -P ./$PRE
RUN tar -zxvf ./$PRE/libneo4j-client-1.2.1.tar.gz -C $PRE/neo
RUN cd $PRE/neo/libneo4j-client-1.2.1 && sudo ./configure --disable-tools && sudo make clean check && sudo make install

#Get the ZMQ Dependencies
RUN cd /tmp && git clone git://github.com/jedisct1/libsodium.git && cd libsodium && git checkout e2a30a && ./autogen.sh && ./configure && make check && make install && ldconfig
RUN wget https://github.com/zeromq/zeromq4-1/releases/download/v4.1.4/zeromq-4.1.4.tar.gz

#Build & Install ZMQ

#Unzip the ZMQ Directories
RUN tar -xvzf zeromq-4.1.4.tar.gz

#Configure, make, & install
RUN cd ./zeromq-4.1.4 && ./configure && make && make install

#Run ldconfig to ensure that ZMQ is on the linker path
RUN ldconfig

#Get the ZMQ C++ Bindings
RUN git clone https://github.com/zeromq/cppzmq.git

#Get ZMQ C++ Header files into include path
RUN cp cppzmq/zmq.hpp /usr/local/include
RUN cp cppzmq/zmq_addon.hpp /usr/local/include

#Get the Eigen Dependency
RUN wget http://bitbucket.org/eigen/eigen/get/3.2.8.tar.bz2

#Unzip the Eigen directories
RUN tar -vxjf 3.2.8.tar.bz2
RUN mkdir $PRE/eigen
RUN mv ./eigen-eigen* $PRE/eigen

#Get the RapidJSON Dependency
RUN git clone https://github.com/miloyip/rapidjson.git

#Move the RapidJSON header files to the include path
RUN cp -r rapidjson/include/rapidjson/ /usr/local/include

#Move the Eigen files
RUN sudo cp -r $PRE/eigen/eigen*/Eigen /usr/local/include

#hayai, for compiling benchmarks
RUN apt-add-repository -y ppa:bruun/hayai

#Update the apt-get cache
RUN apt-get update

#Install Hayai
RUN apt-get install -y libhayai-dev

#Ensure we have access to the Protocol Buffer Interfaces
RUN mkdir $PRE/interfaces/
RUN git clone https://github.com/AO-StreetArt/DvsInterface.git $PRE/interfaces
RUN cd $PRE/interfaces && sudo make install

#Pull the project source from github
RUN git clone https://github.com/AO-StreetArt/AOSharedServiceLibrary.git

#Install the shared service library
RUN cd AOSharedServiceLibrary && make && make install

RUN git clone https://github.com/AO-StreetArt/CrazyIvan.git

# Build the Project & Unit Tests
RUN cd CrazyIvan && make && make test

# Execute Unit Tests
RUN cd CrazyIvan && ./configuration_test -config-file=test/test.properties

#Expose some of the default ports
EXPOSE 22
EXPOSE 8091
EXPOSE 8092
EXPOSE 8093
EXPOSE 11210
EXPOSE 12345

#Expose the 5000 port range for DVS Services
EXPOSE 5000-5999

#Build & Start up Crazy Ivan with script as entry point
ENTRYPOINT ["CrazyIvan/crazy_ivan"]
