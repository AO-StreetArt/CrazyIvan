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

#include "user_device_interface.h"
#include "transform_interface.h"

#ifndef SRC_MODEL_INCLUDE_SCENE_INTERFACE_H_
#define SRC_MODEL_INCLUDE_SCENE_INTERFACE_H_

// Stores the data for a single scene
class SceneInterface {
 public:
  virtual ~SceneInterface() {}
  // Setters
  virtual void set_key(std::string new_key) = 0;
  virtual void set_name(std::string new_name) = 0;
  virtual void set_region(std::string new_region) = 0;
  virtual void set_latitude(double new_lat) = 0;
  virtual void set_longitude(double new_long) = 0;
  virtual void set_distance(double new_dist) = 0;
  virtual void set_active(bool new_active) = 0;

  // Getters
  virtual std::string get_key() const = 0;
  virtual std::string get_name() const = 0;
  virtual std::string get_region() const = 0;
  virtual double get_latitude() const = 0;
  virtual double get_longitude() const = 0;
  virtual double get_distance() const = 0;
  virtual bool active() const = 0;

  // DeviceList
  virtual void add_device(UserDeviceInterface *d) = 0;
  virtual int num_devices() const = 0;
  virtual UserDeviceInterface* get_device(int index) const = 0;
  virtual std::vector<UserDeviceInterface*> get_devices() const = 0;

  // Transform
  virtual TransformInterface* get_scene_transform() const = 0;
  virtual bool has_transform() const = 0;
  virtual void set_transform(TransformInterface *trns) = 0;

  // Scene Asset ID List
  virtual void add_asset(std::string new_asset) = 0;
  virtual int num_assets() const = 0;
  virtual std::string get_asset(int index) const = 0;
  virtual std::vector<std::string> get_assets() const = 0;

  // Tag list
  virtual void add_tag(std::string new_tag) = 0;
  virtual int num_tags() const = 0;
  virtual std::string get_tag(int index) const = 0;
  virtual std::vector<std::string> get_tags() const = 0;

  // Convinience method to log the contents of the scene
  virtual void print() = 0;
};

#endif  // SRC_MODEL_INCLUDE_SCENE_INTERFACE_H_
