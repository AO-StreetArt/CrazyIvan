################################################################

# Dockerfile to build Crazy Ivan Container Images
# Based on Ubuntu-ssh

################################################################

#Based on Ubuntu 16.04
FROM ubuntu:16.04

#Set the Maintainer
MAINTAINER Alex Barry

#Set up front end
ENV DEBIAN_FRONTEND noninteractive

ADD ./scripts/linux/deb/build_deps.sh .

#Setup basic environment tools
RUN ./build_deps.sh

ENV LD_LIBRARY_PATH=/usr/local/lib

#Add the project source to the current directory in the container
ADD . CrazyIvan/

# Build the Project & Unit Tests
RUN cd CrazyIvan && make && make test

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
