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

#include <string>

#include "rapidjson/document.h"

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

#include "aossl/neo4j/include/neo4j_interface.h"
#include "aossl/uuid/include/uuid_interface.h"

#ifndef SRC_PROC_PROCESSOR_INCLUDE_BASE_MESSAGE_PROCESSOR_H_
#define SRC_PROC_PROCESSOR_INCLUDE_BASE_MESSAGE_PROCESSOR_H_

// Base Message Processor maintains the processor components
// and exposes them for child classes to utilize
class BaseMessageProcessor {
  // Internal Variables
  Neo4jInterface *n = NULL;
  Neo4jComponentFactory *neo_factory = NULL;
  ConfigurationManager *config = NULL;
  uuidInterface *ugen = NULL;
  QueryHelperInterface *qh = NULL;
  SceneListFactory slfactory;
  SceneFactory sfactory;
  TransformFactory tfactory;
  UserDeviceFactory udfactory;
  QueryHelperFactory qfactory;

 public:
  // Access internal components
  Neo4jInterface* get_neo4j_interface() {return n;}
  Neo4jComponentFactory* get_neo4j_factory() {return neo_factory;}
  ConfigurationManager* get_config_manager() {return config;}
  QueryHelperInterface* get_query_helper() {return qh;}
  SceneListFactory get_slfactory() {return slfactory;}
  SceneFactory get_sfactory() {return sfactory;}
  UserDeviceFactory get_udfactory() {return udfactory;}
  TransformFactory get_tfactory() {return tfactory;}

  // Helper Methods
  // Generate a new UUID
  void create_uuid(std::string &out_string);
  // Create a response in the form of a scene
  SceneListInterface* build_response_scene(int msg_type, int err_code, \
    std::string err_msg, std::string tran_id, std::string scene_id);
  // Create a response in the form of a string
  void build_string_response(int msg_type, int err_code, std::string err_msg, \
    std::string tran_id, std::string scene_id, std::string &out_string);
  void build_string_response(int msg_type, int err_code, std::string err_msg, \
    std::string tran_id, std::string scene_id, std::string dev_id, \
    TransformInterface *t, std::string &out_string);

  // Ping the Crazy Ivan Instance
  inline ProcessResult* process_ping_message(SceneListInterface *obj_msg) \
    {return new ProcessResult;}

  // Kill the Crazy Ivan Instance
  inline ProcessResult* process_kill_message(SceneListInterface *obj_msg) \
    {return new ProcessResult;}

  // Constructor
  inline BaseMessageProcessor(Neo4jComponentFactory *nf, \
    Neo4jInterface *neo4j, \
    ConfigurationManager *con, uuidInterface *u) {
    n = neo4j;
    r = rd;
    config = con;
    ugen = u;
    neo_factory = nf;
    qh = qfactory.build_query_helper(n, neo_factory, con);
  }

  // Destructor
  virtual inline ~BaseMessageProcessor() {
    delete qh;
  }
};

#endif  // SRC_PROC_PROCESSOR_INCLUDE_BASE_MESSAGE_PROCESSOR_H_
