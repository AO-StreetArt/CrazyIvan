//This defines some of the global variables that need to be accessed
//Across different files.  These are either numeric constants or
//singletons which form the backbone of the service.

#include "aossl/commandline/include/commandline_interface.h"
#include "aossl/commandline/include/factory_cli.h"

#include "aossl/neo4j/include/neo4j_interface.h"
#include "aossl/neo4j/include/factory_neo4j.h"

#include "aossl/logging/include/logging_interface.h"
#include "aossl/logging/include/factory_logging.h"

#include "aossl/redis/include/redis_interface.h"
#include "aossl/redis/include/factory_redis.h"

#include "aossl/uuid/include/uuid_interface.h"
#include "aossl/uuid/include/factory_uuid.h"

#include "aossl/zmq/include/zmq_interface.h"
#include "aossl/zmq/include/factory_zmq.h"

#include "message_processor.h"
#include "configuration_manager.h"
#include "ivan_log.h"
#include "scene.h"
#include "Scene.pb.h"

#include <unordered_map>

#ifndef GLOBALS
#define GLOBALS

//Set up an Obj3 pointer to hold the currently translated document information
extern Scene *translated_object;
extern Scene *resp;

//Globals defined within this service
extern ConfigurationManager *cm;
extern MessageProcessor *processor;

//Globals from the AO Shared Service Library
extern Neo4jInterface *neo;
extern RedisInterface *xRedis;
extern uuidInterface *ua;
extern Zmqio *zmqi;
extern CommandLineInterface *cli;

//Global Factory Objects
extern CommandLineInterpreterFactory *cli_factory;
extern Neo4jComponentFactory *neo4j_factory;
extern RedisComponentFactory *redis_factory;
extern uuidComponentFactory *uuid_factory;
extern ZmqComponentFactory *zmq_factory;
extern LoggingComponentFactory *logging_factory;

//Shutdown the application
inline void shutdown()
{
  //Delete objects off the heap
  if (processor) {
    delete processor;
  }
  if (xRedis) {
    delete xRedis;
  }
  if (neo) {
    delete neo;
  }
  if (zmqi) {
    delete zmqi;
  }
  if (cm) {
    delete cm;
  }
  if (ua) {
    delete ua;
  }
  if (cli) {
    delete cli;
  }

  if(!resp) {main_logging->debug("No response object active at the time of shutdown");}
  else {delete resp;}

  if (!translated_object) {main_logging->debug("No translated object active at time of shutdown");}
  else {delete translated_object;}

  shutdown_logging_submodules();
  if (logging) {
    delete logging;
  }

  //Shut down protocol buffer library
  google::protobuf::ShutdownProtobufLibrary();

  if (cli_factory) {
    delete cli_factory;
  }
  if (neo4j_factory) {
    delete neo4j_factory;
  }
  if (redis_factory) {
    delete redis_factory;
  }
  if (uuid_factory) {
    delete uuid_factory;
  }
  if (zmq_factory) {
    delete zmq_factory;
  }
  if (logging_factory) {
    delete logging_factory;
  }
}

#endif
