.. _configuration:

Configuration
=============

Properties File
---------------

Crazy Ivan can be configured via a properties file, which has a few
command line options:

-  ``./crazy_ivan`` - This will start Crazy Ivan with the default
   properties file, ivan.properties
-  ``./crazy_ivan -config-file=file.properties`` - This will start Crazy
   Ivan with the properties file, file.properties. Can be combined with
   -log-conf.
-  ``./crazy_ivan -log-conf=logging.properties`` - This will start Crazy
   Ivan with the logging properties file, logging.properties. Can be
   combined with -config-file.

The properties file can be edited in any text editor.

Consul
------

Crazy Ivan can also be configured via a Consul Connection, in which we
must specify the address of the consul agent, and the ip & port of the
Inbound ZeroMQ Connection.

-  ``./crazy_ivan -consul-addr=localhost:8500 -ip=localhost -port=5555``
   - Start Crazy Ivan, register as a service with consul, and configure
   based on configuration values in Consul, and bind to an internal 0MQ
   port on localhost
-  ``./crazy_ivan -consul-addr=localhost:8500 -ip=tcp://my.ip -port=5555 -log-conf=logging.properties``
   - Start Crazy Ivan, register as a service with consul, and configure
   based on configuration values in Consul. Bind to an external 0MQ port
   on tcp://my.ip, and configure from the logging configuration file,
   logging.properties.

We can also use both a properties file and a Consul connection, in which case
the properties file is used to define the ip and port of the inbound ZeroMQ connection,
while Consul is used for registration and all other configuration retrieval.

-  ``./crazy_ivan -consul-addr=localhost:8500 -config-file=file.properties``

When configuring from Consul the keys of the properties file are equal
to the expected keys in Consul.

Logging
-------

The Logging Configuration File can also be edited with a text file, and
the documentation for this can be found [here]
(http://log4cpp.sourceforge.net/api/classlog4cpp\_1\_1PropertyConfigurator.html).
Note that logging configuration is not yet in Consul, and always exists
in a properties file.

Two logging configuration files are provided, one for logging to the
console and to a file (log4cpp.properties), and another to log to syslog
and to a file (log4cpp\_syslog.properties). Both show all of the logging
modules utilized by Crazy Ivan during all phases of execution, and all
of these should be configured with the same names (for example,
log4cpp.category.main).

Crazy Ivan is built with many different logging modules, so that
configuration values can change the log level for any given module, the
log file of any given module, or shift any given module to a different
appender or pattern entirely. These modules should always be present
within configuration files, but can be configured to suit the particular
deployment needs.

Startup
-------

Crazy Ivan can be started with an option to wait for a specified number of
seconds prior to looking for configuration values and opening up for requests.
This is particularly useful when used with orchestration providers, in order
to ensure that other components are properly started (in particular, in order
to allow time for Consul to be populated with default configuration values).

-  ``./crazy_ivan -wait=5`` - This will start Crazy Ivan with the default
   properties file, and wait 5 seconds before starting.

Configuration Key-Value Variables
---------------------------------

Below you can find a summary of the options in the Properties File or
Consul Key-Value Store:

DB
~~

-  DB\_ConnectionString - The string used to connect to the Neo4j
   instance (example: neo4j://neo4j:neo4j@localhost:7687)

0MQ
~~~

-  0MQ\_InboundConnectionString - The connectivity string for the
   inbound 0MQ Port

Redis Connection
~~~~~~~~~~~~~~~~

-  RedisConnectionString - We have a list of Redis Connections in our
   cluster in the format "ip--port--password--poolsize--timeout--role"

Behavior
~~~~~~~~

-  StampTransactionId - True to stamp Transaction ID's on messages,
   False if not. Transaction ID's are passed on Inbound Responses and
   Outbound messages, in order to link the two together.
-  AtomicTransactions - True to enforce all instances of Crazy Ivan to
   execute on an individual scene one at a time.
-  Data_Format_Type - JSON to accept JSON messages, protobuf to
   accept protocol buffer messages

:ref:`Go Home <index>`
