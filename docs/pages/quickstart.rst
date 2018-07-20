.. _quickstart:

Getting Started with CrazyIvan
==============================

:ref:`Go Home <index>`

Docker
------

Using the Crazy Ivan Docker image is as simple as:

``docker run aostreetart/crazyivan:v2``

However, we also need a running instance of Neo4j to do anything interesting.  To
get you up and running quickly, a Docker Compose file is provided.  To start up
a Neo4j instance and a Crazy Ivan instance, simply run the following from the
'compose/min' folder:

``docker-compose up``

Alternatively, you can deploy the stack with Docker Swarm using:

``docker stack deploy --compose-file compose/min/docker-compose.yml ivan-stack``

Once the services have started, test them by hitting Ivan's healthcheck endpoint:

``curl http://localhost:8765/health``

The Transaction (HTTP) API is available on port 8765, and the Event (UDP) API
is available on port 8764.  Keep in mind that this is not a secure deployment,
but is suitable for exploring the :ref:`Crazy Ivan API <api_index>`.

Building from Source
--------------------

The recommended system for development of CrazyIvan is either
Ubuntu 18.04 or CentOS7.  You will need gcc 6.0 or greater and gnu make
installed to successfully compile the program.

* Ubuntu

``sudo apt-get install gcc-6 g++-6``

``export CC=gcc-6``

``export CXX=g++-6``

* Redhat

https://www.softwarecollections.org/en/scls/rhscl/devtoolset-6/

Next, you'll need to clone the repository and run the build_deps script.
This will install all of the required dependencies for Crazy Ivan, and may take
a while to run.

``git clone https://github.com/AO-StreetArt/CrazyIvan.git``

``mkdir crazyivan_deps``

``cp CrazyIvan/scripts/deb/build_deps.sh crazyivan_deps/build_deps.sh``

``cd crazyivan_deps``

``sudo ./build_deps.sh``

You will also need to ensure that the POCO dependency is on the linker path,
which can be done with:

`export LD_LIBRARY_PATH="/usr/local/lib:$LD_LIBRARY_PATH"`

Now, we can build Crazy Ivan:

``cd ../CrazyIvan``

``make``

This will result in creation of the crazy_ivan executable, which we can run
with the below command:

``./crazy_ivan``

When not supplied with any command line parameters, CrazyIvan will look for an app.properties file to start from.

You may also build the test executable in the tests/ directory with:

``make tests``

In order to run CrazyIvan from a properties file, you will need:

-  A Neo4j Server installed locally.  Instructions
   can be found at https://neo4j.com/developer/get-started/

Neo4j can be started via a Docker image:

`docker run -d --publish=7474:7474 --publish=7687:7687 --env=NEO4J_AUTH=none --volume=$HOME/neo4j/data:/data --name=database neo4j`

You can move on to explore the :ref:`Crazy Ivan API <api_index>`, or
check out the :ref:`Configuration Section <configuration>` for more details
on the configuration options available when starting CrazyIvan.

Shutdown
--------
Shutdown of Crazy Ivan can be initiated with a kill or interrupt signal to the
main thread.  However, at least one udp message must be recieved in order to
successfully shut down the main event thread.  You can send one with:

`echo "kill" | nc -u $(ip addr show eth0 | grep -Po 'inet \K[\d.]+') 8764`

Replacing 'eth0' with your network device, if necessary.
