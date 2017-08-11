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

#include "include/ivan_utils.h"

#include "include/transform_interface.h"
#include "include/user_device_interface.h"
#include "include/scene_interface.h"

#include "include/scene_list_interface.h"

#ifndef SRC_PROC_QUERY_INCLUDE_QUERY_HELPER_INTERFACE_H_
#define SRC_PROC_QUERY_INCLUDE_QUERY_HELPER_INTERFACE_H_

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
  TransformInterface *transform = NULL;
  bool result_flag;
  void clear() {result_flag = false; transform->clear();}
  ~SceneTransformResult() {if (transform) delete transform;}
};

// The class contains helper methods for working with
// Scene Data in Neo4j
class QueryHelperInterface {
 public:
  virtual ~QueryHelperInterface() {}

  // Does the scene exist in the DB?
  virtual bool scene_exists(std::string inp_key) = 0;

// -----------------------Scene-Device Links--------------------------------- //

  // Determine if the given user device is registered to the given scene
  virtual bool is_ud_registered(std::string inp_string, \
    std::string inp_device) = 0;

  // Get scenes that this user device is registered to
  // Collect the User Device and Transformation within the scenes returned
  virtual SceneListInterface* get_registrations(std::string inp_device) = 0;

  // Update the transformation between scene and device
  virtual bool update_device_registration(std::string dev_id, \
    std::string scene_id, TransformInterface *transform) = 0;

  // Create a registration link in the DB
  virtual void register_device_to_scene(std::string device_id, \
    std::string scene_id, TransformInterface *transform) = 0;

  // Remove a device from a scene
  virtual void remove_device_from_scene(std::string device_id, \
    std::string scene_id) = 0;

// ------------------------Scene-Scene Links--------------------------------- //

  // Get the path between two scenes
  virtual int get_scene_link(std::string scene1, std::string scene2) = 0;

  // Create a link between scenes
  virtual void create_scene_link(std::string s1_key, std::string s2_key, \
    TransformInterface *new_trans) = 0;

  // Update a link between scenes
  virtual void update_scene_link(std::string s1_key, std::string s2_key, \
    TransformInterface *new_trans) = 0;

// ----------------------------Algorithms------------------------------------ //

  // Use a Device registered to multiple scenes to create Scene-Scene
  // Transform Links in the DB
  // As an input, we expect a set of Scenes and User Device pairs
  // which are then processed to generate the Scene-Scene links
  virtual void \
    process_UDUD_transformation(SceneListInterface *registered_scenes, \
    SceneListInterface *obj_msg) = 0;

  // Try to find a path from one scene to the next and calculate
  // the resulting transform
  virtual SceneTransformResult \
    calculate_scene_scene_transform(std::string scene_id1, \
    std::string scene_id2) = 0;
};

#endif  // SRC_PROC_QUERY_INCLUDE_QUERY_HELPER_INTERFACE_H_
