.. _quickstart:

Getting Started with CrazyIvan
==============================

:ref:`Go Home <index>`

Docker
------

Using the Crazy Ivan Docker image is as simple as:

.. code-block:: bash

   docker run --publish=8766:8766 --publish=8764:8764/udp aostreetart/crazyivan:v2

However, we also need a running instance of Neo4j to do anything interesting.  To
get you up and running quickly, a Docker Compose file is provided.  To start up
a Neo4j instance and a Crazy Ivan instance, simply run the following from the
'compose/min' folder:

.. code-block:: bash

   docker-compose up

Alternatively, you can deploy the stack with Docker Swarm using:

.. code-block:: bash

   docker stack deploy --compose-file compose/min/docker-compose.yml ivan-stack

Once the services have started, test them by hitting Ivan's healthcheck endpoint:

.. code-block:: bash

   curl http://localhost:8766/health

The Transaction (HTTP) API is available on port 8766, and the Event (UDP) API
is available on port 8764.  Keep in mind that this is not a secure deployment,
but is suitable for exploring the :ref:`Crazy Ivan API <api_index>`.

You may also continue on to the discussion of :ref:`How to Use Crazy Ivan <use>`.

Shutdown
--------
Shutdown of Crazy Ivan can be initiated with a kill or interrupt signal to the
container, or with 'docker stop'.  However, at least one udp message must be
received afterwards in order to successfully shut down the main event thread.
You can send one with:

.. code-block:: bash

   echo "kill" | nc -u $(ip addr show eth0 | grep -Po 'inet \K[\d.]+') 8764

Replacing 'eth0' with your network device, if necessary.

Latest Release
--------------

Download and unzip the latest release file from https://github.com/AO-StreetArt/CrazyIvan/releases.

Once you have done this, you can run the easy_install script with the -d option
to install dependencies and the Crazy Ivan executable.  Alternatively, you can
simply run the install_deps.sh script from the scripts/ folder, and then run
the crazy_ivan executable from the main release folder.

.. code-block:: bash

   ./crazy_ivan

In order to run CrazyIvan, you will need a Neo4j Server installed locally.
Instructions can be found at https://neo4j.com/developer/get-started/, or Neo4j
can be started via a Docker image:

.. code-block:: bash

   docker run -d --publish=7474:7474 --publish=7687:7687 --env=NEO4J_AUTH=none --volume=$HOME/neo4j/data:/data --name=database neo4j

Either way, the default connection for CrazyIvan will connect without authentication.

You can move on to explore the :ref:`Crazy Ivan API <api_index>`, or
check out the :ref:`Configuration Section <configuration>` for more details
on the configuration options available when starting CrazyIvan.

You may also continue on to the discussion of :ref:`How to Use Crazy Ivan <use>`.



Building from Source
--------------------

The recommended system for development of CrazyIvan is either
Ubuntu 18.04 or CentOS7.  You will need gcc 6.0 or greater and gnu make
installed to successfully compile the program.

* Ubuntu

.. code-block:: bash

   sudo apt-get install gcc-6 g++-6
   export CC=gcc-6
   export CXX=g++-6

* Redhat

https://www.softwarecollections.org/en/scls/rhscl/devtoolset-6/

Next, you'll need to clone the repository and run the build_deps script.
This will install all of the required dependencies for Crazy Ivan, and may take
a while to run.

.. code-block:: bash

   git clone https://github.com/AO-StreetArt/CrazyIvan.git
   mkdir crazyivan_deps
   cp CrazyIvan/scripts/deb/build_deps.sh crazyivan_deps/build_deps.sh
   cd crazyivan_deps
   sudo ./build_deps.sh

You will also need to ensure that the POCO dependency is on the linker path,
which can be done with:

.. code-block:: bash

   export LD_LIBRARY_PATH="/usr/local/lib:$LD_LIBRARY_PATH"

Now, we can build Crazy Ivan:

.. code-block:: bash

   cd ../CrazyIvan
   make

This will result in creation of the crazy_ivan executable, which we can run
with the below command:

.. code-block:: bash

   ./crazy_ivan

When not supplied with any command line parameters, CrazyIvan will look for an app.properties file to start from.

You may also build the test executable in the tests/ directory with:

.. code-block:: bash

   make tests

In order to run CrazyIvan from a properties file, you will need:

-  A Neo4j Server installed locally.  Instructions
   can be found at https://neo4j.com/developer/get-started/

Neo4j can be started via a Docker image:

.. code-block:: bash

   docker run -d --publish=7474:7474 --publish=7687:7687 --env=NEO4J_AUTH=none --volume=$HOME/neo4j/data:/data --name=database neo4j

Either way, the default connection for CrazyIvan will connect without authentication.

You can move on to explore the :ref:`Crazy Ivan API <api_index>`, or
check out the :ref:`Configuration Section <configuration>` for more details
on the configuration options available when starting CrazyIvan.

You may also continue on to the discussion of :ref:`How to Use Crazy Ivan <use>`.

Shutdown
--------
Shutdown of Crazy Ivan can be initiated with a kill or interrupt signal to the
main thread.  However, at least one udp message must be received afterwards
in order to successfully shut down the main event thread.  You can send one with:

.. code-block:: bash

   echo "kill" | nc -u $(ip addr show eth0 | grep -Po 'inet \K[\d.]+') 8764

Replacing 'eth0' with your network device, if necessary.
