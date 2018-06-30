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

#include "app/include/ivan_utils.h"

#include "model/include/transform_interface.h"
#include "model/include/transform_factory.h"
#include "model/include/user_device_interface.h"
#include "model/include/user_device_factory.h"
#include "model/include/scene_interface.h"
#include "model/include/scene_factory.h"

#include "api/include/scene_list_interface.h"
#include "api/include/scene_list_factory.h"

#include "proc/query/include/query_helper_interface.h"
#include "proc/query/include/query_helper_factory.h"
#include "proc/processor/include/processor_interface.h"

#include "neocpp/connection/interface/neo4j_interface.h"
#include "neocpp/connection/impl/libneo4j_factory.h"

#include "aossl/core/include/kv_store_interface.h"
#include "aossl/uuid/include/uuid_interface.h"

#include "crud_message_processor.h"

#ifndef SRC_PROC_PROCESSOR_INCLUDE_MESSAGE_PROCESSOR_H_
#define SRC_PROC_PROCESSOR_INCLUDE_MESSAGE_PROCESSOR_H_

// The class containing core logic for CrazyIvan
// Accepts an Scene (assuming it is an inbound message)
// And performs any and all processing on it,
// Includes DB Interactions, and any necessary calculations
class MessageProcessor : public CrudMessageProcessor, \
    public ProcessorInterface {
  std::string cluster_name;
 public:
  // Constructor
  MessageProcessor(Neocpp::LibNeo4jFactory *nf, Neocpp::Neo4jInterface *neo4j, \
    AOSSL::KeyValueStoreInterface *con, AOSSL::UuidInterface *u, std::string cluster) : \
    CrudMessageProcessor(nf, neo4j, con, u) {cluster_name.assign(cluster);}

  // Destructor
  ~MessageProcessor() {}
  // Process a batch create message
  ProcessResult* process_create_message(SceneListInterface *obj_msg) \
    {return CrudMessageProcessor::process_create_message(obj_msg);}

  // Process a batch update message
  ProcessResult* process_update_message(SceneListInterface *obj_msg) \
    {return CrudMessageProcessor::process_update_message(obj_msg);}

  // Process a query message
  ProcessResult* process_query_message(SceneListInterface *obj_msg) \
    {return CrudMessageProcessor::process_retrieve_message(obj_msg);}

  // Process a batch delete message
  ProcessResult* process_delete_message(SceneListInterface *obj_msg) \
    {return CrudMessageProcessor::process_delete_message(obj_msg);}

  // Process a device retrieval message
  ProcessResult* process_device_get_message(SceneListInterface *obj_msg) \
    {return CrudMessageProcessor::process_device_get_message(obj_msg);}

// -------------------------------------------------------------------------- //
// -------------------------Device Registration------------------------------ //
  // Register a device to a scene
  // Predict a coordinate transform for the device
  // and pass it back in the response
  ProcessResult* process_registration_message(SceneListInterface *obj_msg);

  // Remove a device from a scene
  // If we are removing the last user device from a scene,
  // then remove the scene and try to move any paths that pass through the scene
  ProcessResult* process_deregistration_message(SceneListInterface *obj_msg);

  // Align an objects transformation with a scene
  // If the object is a member of other scenes:
  //  -If a coordinate system transformation doesn't exist, then create one
  //  -If a coordinate system transformation exists, then update it
  ProcessResult* process_device_alignment_message(SceneListInterface *obj_msg);

};

#endif  // SRC_PROC_PROCESSOR_INCLUDE_MESSAGE_PROCESSOR_H_
