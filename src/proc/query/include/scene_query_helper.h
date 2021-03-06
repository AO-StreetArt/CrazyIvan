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

#include "app/include/ivan_utils.h"

#include "model/include/transform_interface.h"
#include "model/include/transform_factory.h"
#include "model/include/user_device_interface.h"
#include "model/include/user_device_factory.h"
#include "model/include/scene_interface.h"
#include "model/include/scene_factory.h"

#include "api/include/scene_list_interface.h"
#include "api/include/scene_list_factory.h"

#include "neocpp/connection/interface/neo4j_interface.h"
#include "neocpp/connection/impl/libneo4j_factory.h"

#include "aossl/core/include/kv_store_interface.h"
#include "aossl/uuid/include/uuid_interface.h"

#include "device_query_helper.h"
#include "query_helper_interface.h"

#include "Poco/Logger.h"

#ifndef SRC_PROC_QUERY_INCLUDE_SCENE_QUERY_HELPER_H_
#define SRC_PROC_QUERY_INCLUDE_SCENE_QUERY_HELPER_H_

// The class contains helper methods for working with
// Scene Data in Neo4j
// The Scene Query Helper deals with individual scenes, and links between them
class SceneQueryHelper : public DeviceQueryHelper {
 public:
  inline SceneQueryHelper(Neocpp::Neo4jInterface *neo, Neocpp::LibNeo4jFactory *nf, \
    AOSSL::KeyValueStoreInterface *con) : DeviceQueryHelper(neo, nf, con) {}
  virtual ~SceneQueryHelper() {}

  // Does the scene exist in the DB?
  bool scene_exists(std::string inp_key);

  // Get the path between two scenes
  int get_scene_link(std::string scene1, std::string scene2);

  // Create a link between scenes
  void create_scene_link(std::string s1_key, std::string s2_key, \
    TransformInterface *new_trans);

  // Update a link between scenes
  void update_scene_link(std::string s1_key, std::string s2_key, \
    TransformInterface *new_trans);
};

#endif  // SRC_PROC_QUERY_INCLUDE_SCENE_QUERY_HELPER_H_
