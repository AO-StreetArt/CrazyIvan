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

#include "include/scene.h"

// Copy Constructor
SceneDocument::SceneDocument(const SceneDocument& sd) {
  // Basic Attributes
  key = sd.get_key();
  name = sd.get_name();
  region = sd.get_region();
  SceneData::set_latitude(sd.get_latitude());
  SceneData::set_longitude(sd.get_longitude());
  distance = sd.get_distance();
  is_active = sd.active();
  trns_flag = false;
  //Scene Assets
  for (int i = 0; i < sd.num_assets(); i++) {
    std::string new_asset;
    new_asset.assign(sd.get_asset(i));
    SceneData::add_asset(new_asset);
  }
  // Tags
  for (int i = 0; i < sd.num_tags(); i++) {
    std::string new_tag;
    new_tag.assign(sd.get_tag(i));
    add_tag(new_tag);
  }
  // Scene Transform
  if (sd.has_transform()) {
    trns_flag = true;
    scene_transform = SceneData::create_transform();
    for (int i = 0; i < 3; i++) {
      scene_transform->translate(i, sd.get_scene_transform()->translation(i));
      scene_transform->rotate(i, sd.get_scene_transform()->rotation(i));
    }
  }
  // Devices
  for (int j = 0; j < sd.num_devices(); j++) {
    TransformInterface *new_tran = SceneData::create_transform();
    for (int k = 0; k < 3; k++) {
      if (sd.get_device(j)->has_transform()) {
        new_tran->translate(k, sd.get_device(j)->get_translation(k));
        new_tran->rotate(k, sd.get_device(j)->get_rotation(k));
      }
    }
    UserDeviceInterface *new_dev = \
      SceneData::create_device(sd.get_device(j)->get_key(), new_tran);
    SceneData::add_device(new_dev);
  }
}

// Create a new scene data object from a protocol buffer scene object
SceneDocument::SceneDocument(protoScene::SceneList_Scene scn_data) {
  obj_logging->debug("Converting Scene Data from Protocol Buffer");

  // New variables
  trns_flag = false;

  // Perform the translation
  if (scn_data.has_key()) {
    key = scn_data.key();
    obj_logging->debug(key);
  }
  if (scn_data.has_region()) {
    region = scn_data.region();
    obj_logging->debug(region);
  }
  if (scn_data.has_latitude()) {
    SceneData::set_latitude(scn_data.latitude());
    obj_logging->debug(SceneData::get_latitude());
  } else {set_latitude(-9999.0);}
  if (scn_data.has_longitude()) {
    SceneData::set_longitude(scn_data.longitude());
    obj_logging->debug(SceneData::get_longitude());
  } else {set_longitude(-9999.0);}
  if (scn_data.has_distance()) {
    distance = scn_data.distance();
    obj_logging->debug(distance);
  } else {distance = -1.0;}
  if (scn_data.has_name()) {
    name = scn_data.name();
    obj_logging->debug(name);
  }
  if (scn_data.has_active()) {
    is_active = scn_data.active();
    obj_logging->debug(is_active);
  }
  if (scn_data.has_transform()) {
    scene_transform = SceneData::create_transform(scn_data.transform());
    trns_flag = true;
    obj_logging->debug("Transform added");
  }
  if (scn_data.devices_size() > 0) {
    for (int k=0; k< scn_data.devices_size(); k++) {
      UserDeviceInterface *ud = SceneData::create_device(scn_data.devices(k));
      SceneData::add_device(ud);
      obj_logging->debug("User Device added");
    }
  }
  for (int m = 0; m < scn_data.assets_size(); m++) {
    SceneData::add_asset(scn_data.assets(m));
  }
  for (int n = 0; n < scn_data.tags_size(); n++) {
    add_tag(scn_data.tags(n));
  }
}
