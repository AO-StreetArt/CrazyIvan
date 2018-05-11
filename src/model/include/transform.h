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

#include "rapidjson/document.h"

#include "app/include/ivan_utils.h"

#include "transform_interface.h"

#include "Poco/Logger.h"

#ifndef SRC_MODEL_INCLUDE_TRANSFORM_H_
#define SRC_MODEL_INCLUDE_TRANSFORM_H_

// Transform
// Stores a translation and rotation which is the relationship between the
// scene origin/axis and the device origin/axis
class Transform : public TransformInterface {
  std::vector<double> tran;
  std::vector<double> rot;
  bool tran_flag;
  bool rot_flag;
  const rapidjson::Value *translation_val;
  const rapidjson::Value *rotation_val;

 public:
  Transform();
  ~Transform() {}

  // Translations
  double translation(int index) const {return tran[index];}
  bool has_translation() const {return tran_flag;}
  inline void translate(int index, double amt) {
    tran[index] = tran[index] + amt;
    tran_flag = true;
  }

  // Rotations
  double rotation(int index) const {return rot[index];}
  bool has_rotation() const {return rot_flag;}
  inline void rotate(int index, double amt) {
    rot[index] = rot[index] + amt;
    rot_flag = true;
  }

  // Add transforms together
  void add_transform(TransformInterface *t, bool inverted);
  void add_transform(TransformInterface *t) {add_transform(t, false);}
  // Invert a transform
  void invert();
  // Clear the Transformation
  void clear() {tran.clear(); rot.clear(); tran_flag = false; rot_flag = false;}
  // Print the transform to the logs
  inline void print() {
    if (tran_flag) {
      Poco::Logger::get("Data").debug("{\"Translation\": [%f, %f, %f]}", tran[0], tran[1], tran[2]);
    }
    if (rot_flag) {
      Poco::Logger::get("Data").debug("{\"Rotation\": [%f, %f, %f]}", rot[0], rot[1], rot[2]);
    }
  }
};

#endif  // SRC_MODEL_INCLUDE_TRANSFORM_H_
