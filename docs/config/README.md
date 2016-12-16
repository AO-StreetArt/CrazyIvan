# Configuration
## Properties File

Ex38 can be configured via a properties file, which has a few command line options:

* `./ex38` - This will start Ex38 with the default properties file, lyman.properties
* `./ex38 -config-file=file.properties` - This will start Ex38 with the properties file, file.properties.  Can be combined with -log-conf.
* `./ex38 -log-conf=logging.properties` - This will start Ex38 with the logging properties file, logging.properties.  Can be combined with -config-file.

The properties file can be edited in any text editor.

## Consul

Ex38 can also be configured via a Consul Connection, in which we must specify the address of the consul agent, and the ip & port of the Inbound ZeroMQ Connection.

* `./ex38 -consul-addr=localhost:8500 -ip=localhost -port=5555` - Start Ex38, register as a service with consul, and configure based on configuration values in Consul, and bind to an internal 0MQ port on localhost
* `./ex38 -consul-addr=localhost:8500 -ip=tcp://my.ip -port=5555 -log-conf=logging.properties` - Start Ex38, register as a service with consul, and configure based on configuration values in Consul.  Bind to an external 0MQ port on tcp://my.ip, and configure from the logging configuration file, logging.properties.

When configuring from Consul the keys of the properties file are equal to the expected keys in Consul.

## Logging

The Logging Configuration File can also be edited with a text file, and the documentation for this can be found [here] (http://log4cpp.sourceforge.net/api/classlog4cpp_1_1PropertyConfigurator.html).  Note that logging configuration is not yet in Consul, and always exists in a properties file.

Two logging configuration files are provided, one for logging to the console and to a file (log4cpp.properties), and another to log to syslog and to a file (log4cpp_syslog.properties).  Both show all of the logging modules utilized by Ex38 during all phases of execution, and all of these should be configured with the same names (for example, log4cpp.category.main).

Ex38 is built with many different logging modules, so that configuration values can change the log level for any given module, the log file of any given module, or shift any given module to a different appender or pattern entirely.  These modules should always be present within configuration files, but can be configured to suit the particular deployment needs.

### Greylog
Greylog is a distributed logging solution, which can be used to centralize logging across many instances of Ex38.  In order to send messages to Greylog, we recommend configuring a SyslogAppender in the configuration files, and then following the instructions [here] (https://marketplace.graylog.org/addons/a47beb3b-0bd9-4792-a56a-33b27b567856) to push the messages logged to syslog out to Greylog.

## Configuration Key-Value Variables

Below you can find a summary of the options in the Properties File or Consul Key-Value Store:

### DB
* DB_ConnectionString - The string used to connect to the Mongo instance (example: mongodb://localhost:27017/)
* DB_Name - The database name within Mongo
* DB_HeaderCollectionName - The Collection Name to store header information within Mongo
* DB_DataCollectionName - The Collection Name to store data chunks within Mongo

### 0MQ
* 0MQ_InboundConnectionString - The connectivity string for the inbound 0MQ Port

### Redis Connection
* RedisConnectionString - We have a list of Redis Connections in our cluster in the format "ip--port--password--poolsize--timeout--role"

### Behavior
* DataChunkSize - The maximum size, in bytes, of the largest possible entry into the data collection in Mongo.  Upon saving, data will be chunked to this size and written in separate documents to Mongo.
* StampTransactionId - True to stamp Transaction ID's on messages, False if not.  Transaction ID's are passed on Inbound Responses and Outbound messages, in order to link the two together.
* SendOutboundFailureMsg - True to send outbound messages on errors.  False to suppress outbound messages in the case of an error.
* EnableObjectLocking - True to enable lock & unlock messages from clients, and reject updates from different device ID's.  Smart Updates are required to enforce locking.

# Next
Now that you have configured Ex38, you can move on to the [Architecture] (https://github.com/AO-StreetArt/Ex38/tree/master/docs/arch) section.
