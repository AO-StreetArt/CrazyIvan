.. _quickstart:

Getting Started with CrazyIvan
==============================

:ref:`Go Home <index>`

Docker
------

The easiest way to get started with CrazyIvan is with `Docker <https://docs.docker.com/get-started/>`__

If you do not have Docker installed, please visit the link above to get setup before continuing.

The first thing we need to do is setup the Docker Network that will allow us to communicate between our containers:

``docker network create dvs``

Before we can start CrazyIvan, we need to have a few other programs running first.
Luckily, these can all be setup with Docker as well:

``docker run -d --name=registry --network=dvs consul``

``docker run -d --publish=7474:7474 --publish=7687:7687 --env=NEO4J_AUTH=none --volume=$HOME/neo4j/data:/data --network=dvs --name=database neo4j``


``docker run -i -t -d -p 2181:2181 -p 9092:9092 --env ADVERTISED_PORT=9092 --env ADVERTISED_HOST=queue --name=queue --network=dvs spotify/kafka``

This will start up a single instance each of Neo4j, Kafka, and Consul.  Consul stores our configuration values, so we'll need to set those up.
You can either view the `Consul Documentation <https://www.consul.io/intro/getting-started/ui.html>`__ for information on starting the container with a Web UI, or you can use the commands below for a quick-and-dirty setup:

``docker exec -t registry curl -X PUT -d 'neo4j://graph-db:7687' http://localhost:8500/v1/kv/ivan/DB_ConnectionString``

``docker exec -t registry curl -X PUT -d 'queue:9092' http://localhost:8500/v1/kv/ivan/KafkaBrokerAddress``

``docker exec -t registry curl -X PUT -d 'True' http://localhost:8500/v1/kv/ivan/StampTransactionId``

``docker exec -t registry curl -X PUT -d 'Json' http://localhost:8500/v1/kv/ivan/Data_Format_Type``

Then, we can start up CrazyIvan:

``docker run --name crazyivan --network=dvs -p 5555:5555 -d aostreetart/crazyivan -consul-addr=registry:8500 -ip=localhost -port=5555 -log-conf=CrazyIvan/log4cpp.properties``

This will start an instance of CrazyIvan with the following properties:

- Connected to network 'dvs', which lets us refer to the other containers in the network by name when connecting.
- Listening on localhost port 5555
- Connected to Consul Container

We can open up a terminal within the container by:

``docker exec -i -t crazyivan /bin/bash``

The 'stop_crazyivan.py' script is provided as an easy way to stop CrazyIvan running as
a service.  This can be executed with:

``python stop_crazyivan.py hostname port``

For a more detailed discussion on the deployment of CrazyIvan, please see
the :ref:`Deployment Section <deployment>`
of the documentation.

Using the Latest Release
------------------------

In order to use the latest release, you will still need to start up the
applications used by CrazyIvan, namely Neo4j, Kafka, and Consul.  This can be done
using the docker instructions above, or by installing each to the system manually.
Instructions:
* `Neo4j <https://neo4j.com/developer/get-started/>`__
* `Consul <https://www.consul.io/intro/getting-started/install.html>`__

Then, download the latest release from the `Releases Page <https://github.com/AO-StreetArt/CrazyIvan/releases>`__

Currently, pre-built binaries are available for:

* Ubuntu 16.04
* CentOS7

Unzip/untar the release file and enter into the directory.  Then, we will use the
easy_install.sh script to install CrazyIvan.  Running the below will attempt to install
the dependencies, and then install the CrazyIvan executable:

``sudo ./easy_install.sh -d``

If you'd rather not automatically install dependencies, and only install the executable,
then you can simply leave off the '-d' flag.  Additionally, you may supply
a '-r' flag to uninstall CrazyIvan:

``sudo ./easy_install -r``

Once the script is finished installing CrazyIvan, you can start CrazyIvan with:

``sudo systemctl start crazyivan.service``

The 'stop_crazyivan.py' script is provided as an easy way to stop CrazyIvan running as
a service.  This can be executed with:

``python stop_crazyivan.py hostname port``

Note: The CrazyIvan configuration files can be found at /etc/crazyivan, and the log files
can be found at /var/log/crazyivan.

Building from Source
--------------------

The recommended system for development of CrazyIvan is either
Ubuntu 16.04 or CentOS7.  You will need gcc 5.0 or greater installed to
successfully compile the program.

``git clone https://github.com/AO-StreetArt/CrazyIvan.git``

``mkdir crazyivan_deps``

``cp CrazyIvan/scripts/linux/deb/build_deps.sh crazyivan_deps/build_deps.sh``

``cd crazyivan_deps``

``./build_deps.sh``

You will be asked once for your sudo password.

``cd ../CrazyIvan``

``make``

This will result in creation of the crazy_ivan executable, which we can run
with the below command:

``./crazy_ivan``

When not supplied with any command line parameters, CrazyIvan will look for an ivan.properties file and log4cpp.properties file to start from.

You may also build the test modules with:

``make tests``

In order to run CrazyIvan from a properties file, you will need:

-  You will also need to have a Neo4j Server installed locally.  Instructions
   can be found at https://neo4j.com/developer/get-started/

Continue on to the :ref:`Configuration Section <configuration>` for more details
on the configuration options available when starting CrazyIvan.
