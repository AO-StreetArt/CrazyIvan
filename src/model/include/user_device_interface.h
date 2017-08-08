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

#include "include/transform_interface.h"
#include "include/transform_factory.h"
#include "include/scene_exception.h"

#ifndef SRC_MODEL_INCLUDE_USER_DEVICE_INTERFACE_H_
#define SRC_MODEL_INCLUDE_USER_DEVICE_INTERFACE_H_

// User Device
// Stores a transform (relative to current scene) and key
class UserDeviceInterface {
 public:
  virtual ~UserDeviceInterface() {}

  // Transforms
  virtual void set_transform(TransformInterface *transform) = 0;
  virtual TransformInterface* get_transform() const = 0;
  virtual bool has_transform() const = 0;
  virtual double get_translation(int index) const = 0;
  virtual double get_rotation(int index) const = 0;
  virtual void set_translation(int index, double amt) = 0;
  virtual void set_rotation(int index, double amt) = 0;

  // Keys
  virtual void set_key(std::string new_key) = 0;
  virtual std::string get_key() const = 0;
  // Print
  virtual void print() = 0;
};

#endif  // SRC_MODEL_INCLUDE_USER_DEVICE_INTERFACE_H_
