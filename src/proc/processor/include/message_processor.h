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

#include "include/query_helper.h"
#include "include/processor_interface.h"

#include "rapidjson/document.h"

#include "aossl/neo4j/include/neo4j_interface.h"
#include "aossl/redis/include/redis_interface.h"
#include "aossl/uuid/include/uuid_interface.h"
#include "aossl/zmq/include/zmq_interface.h"

#ifndef SRC_PROC_INCLUDE_MESSAGE_PROCESSOR_H_
#define SRC_PROC_INCLUDE_MESSAGE_PROCESSOR_H_

// The class containing core logic for CrazyIvan
// Accepts an Scene (assuming it is an inbound message)
// And performs any and all processing on it,
// Includes Redis Locks, DB Interactions, and any necessary calculations
class MessageProcessor : public ProcessorInterface {
// -------------------------Internal Variables------------------------------- //
  Neo4jInterface *n = NULL;
  Neo4jComponentFactory *neo_factory = NULL;
  RedisInterface *r = NULL;
  ConfigurationManager *config = NULL;
  RedisLocker *redis_locks = NULL;
  uuidInterface *ugen = NULL;
  QueryHelper *qh = NULL;
  std::string proto_resp = "";
  SceneListInterface *resp_scn = NULL;
  SceneListFactory slfactory;
  SceneFactory sfactory;
  TransformFactory tfactory;
  UserDeviceFactory udfactory;

// --------------------------Internal Methods-------------------------------- //
  std::string create_uuid();
  void get_mutex_lock(std::string obj_key);
  void release_mutex_lock(std::string obj_key);
  void build_response_scene(int msg_type, int err_code, std::string err_msg, \
    std::string tran_id, std::string scene_id);
  void build_string_response(int msg_type, int err_code, std::string err_msg, \
    std::string tran_id, std::string scene_id, int msg_format_type);
  void build_string_response(int msg_type, int err_code, std::string err_msg, \
    std::string tran_id, std::string scene_id, std::string dev_id, \
    TransformInterface *t, int msg_format_type);

// -----------------------------Ping & Kill---------------------------------- //
  // Ping the Crazy Ivan Instance
  ProcessResult* process_ping_message(SceneListInterface *obj_msg);

  // Kill the Crazy Ivan Instance
  ProcessResult* process_kill_message(SceneListInterface *obj_msg);

// ----------------------------CRUD Support---------------------------------- //
  // Create a new scene
  ProcessResult* process_create_message(SceneListInterface *obj_msg);

  // Update the details of a scene entry
  ProcessResult* process_update_message(SceneListInterface *obj_msg);

  // Query for scene data
  ProcessResult* process_retrieve_message(SceneListInterface *obj_msg);

  // Delete a scene
  ProcessResult* process_delete_message(SceneListInterface *obj_msg);

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
  inline MessageProcessor(Neo4jComponentFactory *nf, Neo4jInterface *neo4j, \
    RedisInterface *rd, ConfigurationManager *con, uuidInterface *u) {
    n = neo4j;
    r = rd;
    config = con;
    redis_locks = new RedisLocker(rd);
    ugen = u;
    neo_factory = nf;
    qh = new QueryHelper(n, neo_factory, con);
  }

  // Destructor
  inline ~MessageProcessor() {
    delete redis_locks;
    delete qh;
    if (resp_scn) delete resp_scn;
  }

  // TO-DO: If we wrap this in a method that accepts a const char *
  // TO-DO: and make this threadsafe, then we can turn this into an
  // TO-DO: asynchronous Service

  // Process a message in the form of an Scene
  // In the case of a get message, return the retrieved document
  // In the case of a create message, return the key of the created object
  // Otherwise, return a blank string in the case of success,
  // and "-1" in the case of failure
  inline ProcessResult* process_message(SceneListInterface *obj_msg) {
    // Determine what type of message we have, and act accordingly
    int msg_type = obj_msg->get_msg_type();
    processor_logging->debug("Message Type: " + std::to_string(msg_type));

    if (msg_type == SCENE_CRT) {
      return process_create_message(obj_msg);
    } else if (msg_type == SCENE_UPD) {
      return process_update_message(obj_msg);
    } else if (msg_type == SCENE_GET) {
      return process_retrieve_message(obj_msg);
    } else if (msg_type == SCENE_DEL) {
      return process_delete_message(obj_msg);
    } else if (msg_type == SCENE_ENTER) {
      return process_registration_message(obj_msg);
    } else if (msg_type == SCENE_LEAVE) {
      return process_deregistration_message(obj_msg);
    } else if (msg_type == DEVICE_ALIGN) {
      return process_device_alignment_message(obj_msg);
    } else if (msg_type == KILL) {
      return process_kill_message(obj_msg);
    } else if (msg_type == PING) {
      return process_ping_message(obj_msg);
    } else {
      return new ProcessResult(BAD_MSG_TYPE_ERROR, "Unrecognized Message Type");
    }
  }
};

#endif  // SRC_PROC_INCLUDE_MESSAGE_PROCESSOR_H_
