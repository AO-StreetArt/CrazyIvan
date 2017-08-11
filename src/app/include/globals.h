/*
Apache2 License Notice
Copyright 2017 Alex Barry

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

// This defines some of the global variables that need to be accessed
// Across different files.  These are either numeric constants or
// singletons which form the backbone of the service.

#include <unordered_map>

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

#include "include/processor_interface.h"
#include "include/processor_factory.h"
#include "include/configuration_manager.h"
#include "include/ivan_log.h"
#include "include/scene_list_interface.h"
#include "include/scene_list_factory.h"
#include "include/scene_factory.h"

#ifndef SRC_APP_INCLUDE_GLOBALS_H_
#define SRC_APP_INCLUDE_GLOBALS_H_

// Set up an Obj3 pointer to hold the currently translated document information
extern SceneListInterface *translated_object;
extern SceneListInterface *resp;

// Globals defined within this service
extern ConfigurationManager *cm;
extern ProcessorInterface *processor;

// Globals from the AO Shared Service Library
extern Neo4jInterface *neo;
extern RedisInterface *xRedis;
extern uuidInterface *ua;
extern Zmqio *zmqi;
extern CommandLineInterface *cli;

// Global Factory Objects
extern CommandLineInterpreterFactory *cli_factory;
extern Neo4jComponentFactory *neo4j_factory;
extern RedisComponentFactory *redis_factory;
extern uuidComponentFactory *uuid_factory;
extern ZmqComponentFactory *zmq_factory;
extern LoggingComponentFactory *logging_factory;

extern ProcessorFactory *processor_factory;
extern SceneListFactory *scene_list_factory;
extern SceneFactory *scene_factory;

// Shutdown the application
inline void shutdown() {
  if (!resp) {
    main_logging->debug("No response object active at the time of shutdown");
  } else {delete resp;}

  if (!translated_object) {
    main_logging->debug("No translated object active at time of shutdown");
  } else {delete translated_object;}

  // Delete objects off the heap
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

  shutdown_logging_submodules();
  if (logging) {
    delete logging;
  }

  // Shut down protocol buffer library
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
  if (processor_factory) {
    delete processor_factory;
  }
  if (scene_list_factory) {
    delete scene_list_factory;
  }
}

#endif  // SRC_APP_INCLUDE_GLOBALS_H_
