#include "globals.h"

//Declare our global variables

//Response Object
Scene *resp = NULL;

//Set up an Scene pointer to hold the currently translated document information
Scene *translated_object = NULL;

//UUID Generator
uuidInterface *ua = NULL;

//Configuration Manager
ConfigurationManager *cm = NULL;

//Global Couchbase Admin Object
Neo4jInterface *neo = NULL;

//Smart Update Buffer
RedisInterface *xRedis = NULL;

//Command Line interpreter
CommandLineInterface *cli = NULL;

//Document Manager, highest level object
MessageProcessor *processor = NULL;

//ZMQ Admins
Zmqio *zmqi = NULL;

//Factories
CommandLineInterpreterFactory *cli_factory = NULL;
Neo4jComponentFactory *neo4j_factory = NULL;
RedisComponentFactory *redis_factory = NULL;
uuidComponentFactory *uuid_factory = NULL;
ZmqComponentFactory *zmq_factory = NULL;
LoggingComponentFactory *logging_factory = NULL;
