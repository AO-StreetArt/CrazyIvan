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

#include "ivan_log.h"
#include "ivan_utils.h"
#include "scene.h"

#include "aossl/neo4j/include/neo4j_interface.h"
#include "aossl/neo4j/include/factory_neo4j.h"

#ifndef SRC_INCLUDE_QUERY_HELPER_H_
#define SRC_INCLUDE_QUERY_HELPER_H_

// A Query Exception

// A child class of std::exception
// which holds error information
struct QueryException: public std::exception {
  // An error message passed on initialization
  std::string int_msg;
  const char * int_msg_cstr;

  // Create a Neo4j Exception, and store the given error message
  inline QueryException(std::string msg) {
    int_msg = "Error in Query Helper: " + msg;
    int_msg_cstr = int_msg.c_str();
  }

  QueryException() {}
  ~QueryException() throw() {}
  // Show the error message in readable format
  const char * what() const throw() {
    return int_msg_cstr;
  }
};

struct SceneTransformResult {
  Transform transform;
  bool result_flag;
  void clear() {result_flag = false; transform.clear();}
};

// The class contains helper methods for working with
// Scene Data in Neo4j
class QueryHelper {
Neo4jInterface *n = NULL;
Neo4jComponentFactory *neo_factory = NULL;
SceneTransformResult str;

 public:
  inline QueryHelper(Neo4jInterface *neo, Neo4jComponentFactory *nf) {
    n = neo;
    neo_factory = nf;
  }
  ~QueryHelper() {}

  // Does the scene exist in the DB?
  bool scene_exists(std::string inp_key);

// -----------------------Scene-Device Links--------------------------------- //

  // Determine if the given user device is registered to the given scene
  bool is_ud_registered(std::string inp_string, std::string inp_device);

  // Get scenes that this user device is registered to
  // Collect the User Device and Transformation within the scenes returned
  Scene* get_registrations(std::string inp_device);

  // Update the transformation between scene and device
  bool update_device_registration(std::string dev_id, std::string scene_id, \
    Transform &transform);

  // Create a registration link in the DB
  void register_device_to_scene(std::string device_id, std::string scene_id, \
    Transform &transform);

  // Remove a device from a scene
  void remove_device_from_scene(std::string device_id, std::string scene_id);

// ------------------------Scene-Scene Links--------------------------------- //

  // Get the path between two scenes
  int get_scene_link(std::string scene1, std::string scene2);

  // Create a link between scenes
  void create_scene_link(std::string s1_key, std::string s2_key, \
    Transform new_trans);

  // Update a link between scenes
  void update_scene_link(std::string s1_key, std::string s2_key, \
    Transform new_trans);

// ----------------------------Algorithms------------------------------------ //

  // Use a Device registered to multiple scenes to create Scene-Scene
  // Transform Links in the DB
  // As an input, we expect a set of Scenes and User Device pairs
  // which are then processed to generate the Scene-Scene links
  void process_UDUD_transformation(Scene *registered_scenes, Scene *obj_msg);

  // Try to find a path from one scene to the next and calculate
  // the resulting transform
  SceneTransformResult calculate_scene_scene_transform(std::string scene_id1, \
    std::string scene_id2);
};

#endif  // SRC_INCLUDE_QUERY_HELPER_H_
