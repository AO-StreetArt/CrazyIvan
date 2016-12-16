# Getting Started with Ex38

This guide will cover a few of the fastest means of getting started with a Ex38 development environment.

Regardless of which method you choose to get started, you will need to have Mongo Server, Redis Server, and an available Consul Agent up and running as well.  This can be done via Docker, or manually.

* Please find instructions to install and configure a Mongo Server locally [here] (https://www.mongodb.com)

* To install and configure a Redis Server locally, instructions can be found [here] (http://redis.io/documentation)

* Finally, you should have a Consul Agent installed, please find instructions [here] (https://www.consul.io/docs/index.html).  Note that if we start from a properties file, then no Consul Agent is necessary.

## Docker

This will require Docker installed on the development computer.

The Development Docker image for Ex38-Dev is ready for active use, and can be run with the command:

`docker run --name ex38 -d aostreetart/ex38`

Congratulations, you've got a fully functional build & test environment for Ex38!  You can connect to it with:

`docker exec -i -t ex38 /bin/bash`

For a more detailed discussion on the deployment of Ex38, please see the [Deployment] (https://github.com/AO-StreetArt/Ex38/tree/master/docs/deploy) section of the documentation.  For now, once you are into the Docker instance, you can run the below command to start Ex38.  You can also execute 'make', 'make test', 'make benchmark', etc.

`./ex38`

## Building from Source

While using Docker is a much faster solution, it is sometimes necessary to build from source.

The recommended deployment for development of Ex38 is on Ubuntu 14.04, Debian 7, RHEL 7, or CentOS 7.  Running the below commands in a terminal will create a fully functional build environment.  This will also result in the creation of the ex38 executable, which we can run with the below command:

`./ex38`

This will run the program and look for the default configuration file, ex38.properties.  See the configuration section for more details.

### Ubuntu/Debian

    git clone https://github.com/AO-StreetArt/Ex38.git
    mkdir ex38_deps
    cp Ex38/scripts/linux/deb/build_deps.sh ex38_deps/build_deps.sh
    cd ex38_deps
    ./build_deps.sh

You will be asked once for your sudo password.

    cd ../Ex38
    make

You may also build the test modules with:

`make tests`

### RHEL/CentOS

    git clone https://github.com/AO-StreetArt/Ex38.git
    mkdir ex38_deps
    cp Ex38/scripts/linux/rhel/build_deps.sh ex38_deps/build_deps.sh
    cd ex38_deps
    ./build_deps.sh

You will be asked once for your sudo password.

    cd ../Ex38
    make rhel

You may also build the test modules with:

`make rhel-tests`

#Next
Now that you have a working instance of Ex38, you can move on to the [Configuration] (https://github.com/AO-StreetArt/Ex38/tree/master/docs/config) Section.
