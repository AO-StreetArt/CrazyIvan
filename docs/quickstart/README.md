# Getting Started with CrazyIvan

This guide will cover a few of the fastest means of getting started with a CrazyIvan development environment.

Regardless of which method you choose to get started, you will need to have Mongo Server, Redis Server, and an available Consul Agent up and running as well.  This can be done via Docker, or manually.

* Please find instructions to install and configure a Mongo Server locally [here] (https://www.mongodb.com)

* To install and configure a Redis Server locally, instructions can be found [here] (http://redis.io/documentation)

* Finally, you should have a Consul Agent installed, please find instructions [here] (https://www.consul.io/docs/index.html).  Note that if we start from a properties file, then no Consul Agent is necessary.

## Docker

This will require Docker installed on the development computer.

The Development Docker image for CrazyIvan-Dev is ready for active use, and can be run with the command:

`docker run --name crazyivan -d aostreetart/crazyivan`

Congratulations, you've got a fully functional build & test environment for CrazyIvan!  You can connect to it with:

`docker exec -i -t crazyivan /bin/bash`

For a more detailed discussion on the deployment of CrazyIvan, please see the [Deployment] (https://github.com/AO-StreetArt/CrazyIvan/tree/master/docs/deploy) section of the documentation.  For now, once you are into the Docker instance, you can run the below command to start CrazyIvan.  You can also execute 'make', 'make test', 'make benchmark', etc.

`./crazy_ivan`

## Building from Source

While using Docker is a much faster solution, it is sometimes necessary to build from source.

The recommended deployment for development of CrazyIvan is on Ubuntu 14.04, Debian 7, RHEL 7, or CentOS 7.  Running the below commands in a terminal will create a fully functional build environment.  This will also result in the creation of the crazy_ivan executable, which we can run with the below command:

`./crazy_ivan`

This will run the program and look for the default configuration file, ivan.properties.  See the configuration section for more details.

### Ubuntu/Debian

    git clone https://github.com/AO-StreetArt/CrazyIvan.git
    mkdir ivan_deps
    cp CrazyIvan/scripts/linux/deb/build_deps.sh ivan_deps/build_deps.sh
    cd ivan_deps
    ./build_deps.sh

You will be asked once for your sudo password.

    cd ../CrazyIvan
    make

You may also build the test modules with:

`make tests`

### RHEL/CentOS

    git clone https://github.com/AO-StreetArt/CrazyIvan.git
    mkdir ivan_deps
    cp CrazyIvan/scripts/linux/rhel/build_deps.sh ivan_deps/build_deps.sh
    cd ivan_deps
    ./build_deps.sh

You will be asked once for your sudo password.

    cd ../CrazyIvan
    make rhel

You may also build the test modules with:

`make rhel-tests`

#Next
Now that you have a working instance of CrazyIvan, you can move on to the [Configuration] (https://github.com/AO-StreetArt/CrazyIvan/tree/master/docs/config) Section.
