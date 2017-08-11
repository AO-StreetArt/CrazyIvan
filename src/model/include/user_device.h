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
#include <vector>
#include <algorithm>

#include "include/ivan_log.h"
#include "include/ivan_utils.h"
#include "include/configuration_manager.h"
#include "include/Scene.pb.h"

#include "include/transform_factory.h"
#include "include/transform_interface.h"
#include "include/user_device_interface.h"
#include "include/scene_exception.h"

#ifndef SRC_MODEL_INCLUDE_USER_DEVICE_H_
#define SRC_MODEL_INCLUDE_USER_DEVICE_H_

// User Device
// Stores a transform (relative to current scene) and key
class UserDevice : public UserDeviceInterface {
  TransformInterface *trans = NULL;
  TransformFactory tfactory;
  std::string key = "";
  const char * my_key;
  bool trns_flag = false;
  const rapidjson::Value *key_val;
  const rapidjson::Value *transform_val;

 public:
  // Constructors
  UserDevice(protoScene::SceneList_UserDevice scn_data);
  UserDevice() {trns_flag = false;}
  UserDevice(TransformInterface *transform) \
    {trans = transform; trns_flag = true;}
  UserDevice(std::string new_key) {key = new_key; trns_flag = false;}
  UserDevice(const char * new_key) {my_key = new_key; key.assign(my_key);}
  inline UserDevice(std::string new_key, TransformInterface *transform) {
    key = new_key;
    trans = transform;
    trns_flag = true;
  }
  UserDevice(const UserDevice &ud);
  ~UserDevice() {if (trns_flag) {delete trans;}}

  // Transforms
  inline void set_transform(TransformInterface *transform) {
    trans = transform;
    trns_flag = true;
  }
  TransformInterface* get_transform() const {return trans;}
  double get_translation(int index) const {return trans->translation(index);}
  double get_rotation(int index) const {return trans->rotation(index);}
  inline void set_translation(int index, double amt) {
    if (trns_flag) trans->translate(index, amt);
    else
      {throw SceneException("Attempting to update Device without transform");}
  }
  inline void set_rotation(int index, double amt) {
    if (trns_flag) trans->rotate(index, amt);
    else
      {throw SceneException("Attempting to update Device without transform");}
  }
  inline bool has_transform() const {
    if ((!trans) || (!trns_flag)) return false;
    else
      {return true;}
  }

  // Keys
  void set_key(std::string new_key) {key = new_key;}
  std::string get_key() const {return key;}
  // Print
  inline void print() {
    obj_logging->debug("User Device");
    obj_logging->debug(key);
    if (trns_flag) {trans->print();}
  }
};

#endif  // SRC_MODEL_INCLUDE_USER_DEVICE_H_
