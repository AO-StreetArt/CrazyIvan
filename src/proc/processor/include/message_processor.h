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

#include "include/ivan_log.h"
#include "include/ivan_utils.h"
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

#include "rapidjson/document.h"

#include "aossl/neo4j/include/neo4j_interface.h"
#include "aossl/uuid/include/uuid_interface.h"
#include "aossl/zmq/include/zmq_interface.h"

#include "crud_message_processor.h"

#ifndef SRC_PROC_PROCESSOR_INCLUDE_MESSAGE_PROCESSOR_H_
#define SRC_PROC_PROCESSOR_INCLUDE_MESSAGE_PROCESSOR_H_

// The class containing core logic for CrazyIvan
// Accepts an Scene (assuming it is an inbound message)
// And performs any and all processing on it,
// Includes DB Interactions, and any necessary calculations
class MessageProcessor : public CrudMessageProcessor, \
  public ProcessorInterface {
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

 public:
  // Constructor
  MessageProcessor(Neo4jComponentFactory *nf, Neo4jInterface *neo4j, \
    ConfigurationManager *con, uuidInterface *u) : \
    CrudMessageProcessor(nf, neo4j, rd, con, u) {}

  // Destructor
  ~MessageProcessor() {}

  // Process a message in the form of an Scene
  // In the case of a get message, return the retrieved document
  // In the case of a create message, return the key of the created object
  // In the case of a get message, return string representation of the object
  inline ProcessResult* process_message(SceneListInterface *obj_msg) {
    // Determine what type of message we have, and act accordingly
    int msg_type = obj_msg->get_msg_type();
    processor_logging->debug("Message Type: " + std::to_string(msg_type));

    if (msg_type == SCENE_CRT) {
      return CrudMessageProcessor::process_create_message(obj_msg);
    } else if (msg_type == SCENE_UPD) {
      return CrudMessageProcessor::process_update_message(obj_msg);
    } else if (msg_type == SCENE_GET) {
      return CrudMessageProcessor::process_retrieve_message(obj_msg);
    } else if (msg_type == SCENE_DEL) {
      return CrudMessageProcessor::process_delete_message(obj_msg);
    } else if (msg_type == SCENE_ENTER) {
      return process_registration_message(obj_msg);
    } else if (msg_type == SCENE_LEAVE) {
      return process_deregistration_message(obj_msg);
    } else if (msg_type == DEVICE_ALIGN) {
      return process_device_alignment_message(obj_msg);
    } else if (msg_type == DEVICE_GET) {
      return CrudMessageProcessor::process_device_get_message(obj_msg);
    } else if (msg_type == KILL) {
      return BaseMessageProcessor::process_kill_message(obj_msg);
    } else if (msg_type == PING) {
      return BaseMessageProcessor::process_ping_message(obj_msg);
    } else {
      return new ProcessResult(BAD_MSG_TYPE_ERROR, "Unrecognized Message Type");
    }
  }
};

#endif  // SRC_PROC_PROCESSOR_INCLUDE_MESSAGE_PROCESSOR_H_
