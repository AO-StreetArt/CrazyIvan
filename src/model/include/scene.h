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

// Message accessor for SceneList Protocol Buffer object access
// Message creator for SceneList Protcol Buffer object

#include <string>
#include <vector>
#include <algorithm>

#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

#include "include/ivan_log.h"
#include "include/ivan_utils.h"
#include "include/configuration_manager.h"
#include "include/Scene.pb.h"

#include "scene_interface.h"
#include "scene_data.h"
#include "scene_exception.h"
#include "transform.h"
#include "user_device.h"

#ifndef SRC_MODEL_INCLUDE_SCENE_H_
#define SRC_MODEL_INCLUDE_SCENE_H_

// Stores the data for a single scene
class SceneDocument : public SceneData, public SceneInterface {
  std::string key = "";
  std::string name = "";
  double distance = 0.0;
  bool trns_flag = false;
  TransformInterface* scene_transform;
  const rapidjson::Value *key_val;
  const rapidjson::Value *scene_val;
  const rapidjson::Value *name_val;
  const rapidjson::Value *lat_val;
  const rapidjson::Value *long_val;
  const rapidjson::Value *dist_val;
  const rapidjson::Value *transform_val;
  const rapidjson::Value *devices_val;

 public:
  // Constructors
  SceneDocument() {}
  SceneDocument(protoScene::SceneList_Scene scn_data);
  SceneDocument(const SceneDocument& sd);

  // Destructor
  inline ~SceneDocument() {
    if (trns_flag) delete scene_transform;
  }

  // Setters
  void set_key(std::string new_key) {key = new_key;}
  void set_name(std::string new_name) {name = new_name;}
  void set_latitude(double new_lat) {SceneData::set_latitude(new_lat);}
  void set_longitude(double new_long) {SceneData::set_longitude(new_long);}
  void set_distance(double new_dist) {distance = new_dist;}

  // Getters
  std::string get_key() const {return key;}
  std::string get_name() const {return name;}
  double get_latitude() const {return SceneData::get_latitude();}
  double get_longitude() const {return SceneData::get_longitude();}
  double get_distance() const {return distance;}

  // List
  void add_device(UserDeviceInterface *d) {SceneData::add_device(d);}
  int num_devices() const {return SceneData::num_devices();}
  UserDeviceInterface* get_device(int index) const \
    {return SceneData::get_device(index);}
  std::vector<UserDeviceInterface*> get_devices() const \
    {return SceneData::get_devices();}

  // Transform
  TransformInterface* get_scene_transform() const {return scene_transform;}
  bool has_transform() const {return trns_flag;}
  inline void set_transform(TransformInterface *trns) {
    scene_transform = trns;
    trns_flag = true;
  }

  // Print
  inline void print() {
    obj_logging->debug("Scene Data");
    obj_logging->debug(key);
    obj_logging->debug(name);
    obj_logging->debug(get_latitude());
    obj_logging->debug(get_longitude());
    obj_logging->debug(distance);
    for (int i = 0; i < num_devices(); i++) {get_device(i)->print();}
    if (trns_flag) {scene_transform->print();}
  }
};

#endif  // SRC_MODEL_INCLUDE_SCENE_H_
