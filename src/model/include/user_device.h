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

#include "app/include/ivan_utils.h"

#include "transform_factory.h"
#include "transform_interface.h"
#include "user_device_interface.h"
#include "scene_exception.h"

#include "Poco/Logger.h"

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
  std::string connection_string = "";
  std::string hostname = "";
  int port = 999999;

 public:
  // Constructors
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

  // Connectivity Information
  std::string get_connection_string() const {return connection_string;}
  std::string get_hostname() const {return hostname;}
  int get_port() const {return port;}
  void set_connection_string(std::string new_string) \
    {connection_string = new_string;}
  void set_hostname(std::string new_host) {hostname = new_host;}
  void set_port(int new_port) {port = new_port;}

  // Print
  inline void print() {
    Poco::Logger::get("Data").debug("User Device: %s", key);
    if (trns_flag) {trans->print();}
  }
};

#endif  // SRC_MODEL_INCLUDE_USER_DEVICE_H_
