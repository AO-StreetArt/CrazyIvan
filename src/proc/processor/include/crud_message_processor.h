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

#include "rapidjson/document.h"

#include "aossl/neo4j/include/neo4j_interface.h"
#include "aossl/redis/include/redis_interface.h"
#include "aossl/uuid/include/uuid_interface.h"
#include "aossl/zmq/include/zmq_interface.h"

#include "include/ivan_log.h"
#include "include/ivan_utils.h"
#include "include/redis_locking.h"
#include "include/configuration_manager.h"

#include "include/transform_interface.h"
#include "include/transform_factory.h"
#include "include/user_device_interface.h"
#include "include/user_device_factory.h"
#include "include/scene_interface.h"
#include "include/scene_factory.h"

#include "include/scene_list_interface.h"
#include "include/scene_list_factory.h"

#include "include/query_helper_interface.h"
#include "include/query_helper_factory.h"
#include "include/processor_interface.h"
#include "include/base_message_processor.h"

#ifndef SRC_PROC_PROCESSOR_INCLUDE_CRUD_MESSAGE_PROCESSOR_H_
#define SRC_PROC_PROCESSOR_INCLUDE_CRUD_MESSAGE_PROCESSOR_H_

// The class containing core CRUD logic for CrazyIvan
class CrudMessageProcessor : public BaseMessageProcessor {
 public:
  // Create a new scene
  ProcessResult* process_create_message(SceneListInterface *obj_msg);

  // Update the details of a scene entry
  ProcessResult* process_update_message(SceneListInterface *obj_msg);

  // Query for scene data
  ProcessResult* process_retrieve_message(SceneListInterface *obj_msg);

  // Delete a scene
  ProcessResult* process_delete_message(SceneListInterface *obj_msg);

  ProcessResult* process_device_get_message(SceneListInterface *obj_msg);

  // Constructor
  CrudMessageProcessor(Neo4jComponentFactory *nf, Neo4jInterface *neo4j, \
    RedisInterface *rd, ConfigurationManager *con, uuidInterface *u) : \
    BaseMessageProcessor(nf, neo4j, rd, con, u) {}

  // Destructor
  virtual ~CrudMessageProcessor() {}
};

#endif  // SRC_PROC_PROCESSOR_INCLUDE_CRUD_MESSAGE_PROCESSOR_H_
