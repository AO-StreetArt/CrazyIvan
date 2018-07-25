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

#include "model/include/transform_interface.h"
#include "model/include/transform_factory.h"
#include "model/include/user_device_interface.h"
#include "model/include/user_device_factory.h"
#include "model/include/scene_interface.h"
#include "model/include/scene_factory.h"

#include "api/include/scene_list_interface.h"
#include "api/include/scene_list_factory.h"

#include "proc/query/include/query_helper_interface.h"

#include "neocpp/connection/interface/neo4j_interface.h"
#include "neocpp/connection/impl/libneo4j_factory.h"

#include "aossl/core/include/kv_store_interface.h"
#include "aossl/uuid/include/uuid_interface.h"

#include "scene_query_helper.h"
#include "query_helper_interface.h"

#include "Poco/Logger.h"

#ifndef SRC_PROC_QUERY_INCLUDE_ALGORITHM_QUERY_HELPER_H_
#define SRC_PROC_QUERY_INCLUDE_ALGORITHM_QUERY_HELPER_H_

// The class contains helper methods for working with
// Scene Data in Neo4j
// The Algorithm Query Helper implements the Query Helper Interface, inheriting
// most of the methods from it's children.  The only methods it implements are
// algorithmic methods, built on top of basic queries.
class AlgorithmQueryHelper : public SceneQueryHelper, \
  public QueryHelperInterface {
 public:
  inline AlgorithmQueryHelper(Neocpp::Neo4jInterface *neo, Neocpp::LibNeo4jFactory *nf, \
    AOSSL::KeyValueStoreInterface *con) : SceneQueryHelper(neo, nf, con) {}
  ~AlgorithmQueryHelper() {}

  // Does the scene exist in the DB?
  bool scene_exists(std::string inp_key) \
    {return SceneQueryHelper::scene_exists(inp_key);}

// -----------------------Scene-Device Links--------------------------------- //

  // Determine if the given user device is registered to the given scene
  bool is_ud_registered(std::string inp_string, std::string inp_device) \
    {return DeviceQueryHelper::is_ud_registered(inp_string, inp_device);}

  // Get scenes that this user device is registered to
  // Collect the User Device and Transformation within the scenes returned
  SceneListInterface* get_registrations(std::string inp_device) \
    {return DeviceQueryHelper::get_registrations(inp_device);}

  // Update the transformation between scene and device
  bool update_device_registration(std::string dev_id, \
    std::string scene_id, TransformInterface *transform) \
    {return DeviceQueryHelper::update_device_registration(dev_id, \
    scene_id, transform);}

  // Create a registration link in the DB
  void register_device_to_scene(std::string device_id, \
    std::string scene_id, TransformInterface *transform, bool device_exists, \
    std::string ud_conn_str, std::string ud_host, int ud_port) \
    {DeviceQueryHelper::register_device_to_scene(device_id, scene_id, \
      transform, device_exists, ud_conn_str,ud_host, ud_port);}

  // Remove a device from a scene
  void remove_device_from_scene(std::string device_id, std::string scene_id) \
    {DeviceQueryHelper::remove_device_from_scene(device_id, scene_id);}

// ------------------------Scene-Scene Links--------------------------------- //

  // Get the path between two scenes
  int get_scene_link(std::string scene1, std::string scene2) \
    {return SceneQueryHelper::get_scene_link(scene1, scene2);}

  // Create a link between scenes
  void create_scene_link(std::string s1_key, \
    std::string s2_key, TransformInterface *new_trans) \
    {SceneQueryHelper::create_scene_link(s1_key, s2_key, new_trans);}

  // Update a link between scenes
  void update_scene_link(std::string s1_key, \
    std::string s2_key, TransformInterface *new_trans) \
    {SceneQueryHelper::create_scene_link(s1_key, s2_key, new_trans);}

// ----------------------------Algorithms------------------------------------ //

  // Use a Device registered to multiple scenes to create Scene-Scene
  // Transform Links in the DB
  // As an input, we expect a set of Scenes and User Device pairs
  // which are then processed to generate the Scene-Scene links
  void process_UDUD_transformation(SceneListInterface *registered_scenes, \
    SceneListInterface *obj_msg);

  // Try to find a path from one scene to the next and calculate
  // the resulting transform
  SceneTransformResult* calculate_scene_scene_transform(std::string scene_id1, \
    std::string scene_id2);

// --------------------------Helper Methods---------------------------------- //
  void assign_scene_properties(Neocpp::DbObjectInterface *db_scene, \
    SceneInterface *data) \
    {BaseQueryHelper::assign_scene_properties(db_scene, data);}

  void assign_device_properties(Neocpp::DbObjectInterface *db_device, \
      UserDeviceInterface *data) {
    BaseQueryHelper::assign_device_properties(db_device, data);
  }
  inline void generate_scene_crud_query(std::string key, int crud_op, \
      int op_type, SceneInterface *scn, std::string &query_str) {
    BaseQueryHelper::generate_scene_crud_query(key, crud_op, op_type, scn, query_str);
  }
  inline void generate_scene_query_parameters(std::string key, int crud_op, SceneInterface *scn, \
      std::unordered_map<std::string, Neocpp::Neo4jQueryParameterInterface*> &scene_params) {
    BaseQueryHelper::generate_scene_query_parameters(key, crud_op, scn, scene_params);
  }
};

#endif  // SRC_PROC_QUERY_INCLUDE_ALGORITHM_QUERY_HELPER_H_
