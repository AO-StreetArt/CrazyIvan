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

#include <math.h>

#define GLM_FORCE_RADIANS
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/matrix_transform.hpp>

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
  // 4x4 Matrix to store the transform
  // This is what gets operated on when we combine transforms or
  // invert the transform.  It is the master source of information
  // for the transform while it is stored in memory.
  glm::mat4 transform;
  // Vectors to hold the translation and euler angles.
  // These are sent in messages and stored in Neo4j.
  // we build them on-demand, and only if changes to the
  // internal transform have happenned.
  std::vector<float> tran;
  std::vector<float> rot;
  // Is there a translation element to the transform
  bool tran_flag;
  // Is there a rotation element to the transform
  bool rot_flag;
  // Do we need to update the return vectors when requested?
  bool vectors_need_updating = true;

  const rapidjson::Value *translation_val;
  const rapidjson::Value *rotation_val;

  void update_vectors();

 public:
  Transform();
  ~Transform() {}

  // Transform matrix
  glm::mat4 get_transform_vector() const {return transform;}

  // Translations
  inline double translation(int index) {
    if (vectors_need_updating) {
      update_vectors();
    }
    return static_cast<double>(tran[index]);
  }
  bool has_translation() const {return tran_flag;}
  inline void translate(int index, double amt) {
    glm::mat4 new_transform;
    if (index == 0) {
      new_transform = glm::translate(transform, \
          glm::vec3(static_cast<float>(amt), 0.0f, 0.0f));
    } else if (index == 1) {
      new_transform = glm::translate(transform, \
          glm::vec3(0.0f, static_cast<float>(amt), 0.0f));
    } else if (index == 2) {
      new_transform = glm::translate(transform, \
          glm::vec3(0.0f, 0.0f, static_cast<float>(amt)));
    } else {
      Poco::Logger::get("Data").error("Invalid Index supplied for transform");
    }

    transform = new_transform;
    vectors_need_updating = true;
    tran_flag = true;
  }

  // Rotations
  inline double rotation(int index) {
    if (vectors_need_updating) {
      update_vectors();
    }
    return static_cast<double>(rot[index]);
  }
  bool has_rotation() const {return rot_flag;}
  inline void rotate(int index, double amt) {
    glm::mat4 rotation_matrix;
    if (index == 0) {
      rotation_matrix = glm::eulerAngleYXZ(0.0f, static_cast<float>(amt), 0.0f);
    } else if (index == 1) {
      rotation_matrix = glm::eulerAngleYXZ(static_cast<float>(amt), 0.0f, 0.0f);
    } else if (index == 2) {
      rotation_matrix = glm::eulerAngleYXZ(0.0f, 0.0f, static_cast<float>(amt));
    } else {
      Poco::Logger::get("Data").error("Invalid Index supplied for transform");
    }

    glm::mat4 new_transform = rotation_matrix * transform;
    transform = new_transform;
    vectors_need_updating = true;
    rot_flag = true;
  }

  // Add transforms together
  // applies the supplied transform with LHS matrix multplication
  void add_transform(TransformInterface *t, bool inverted);
  void add_transform(TransformInterface *t) {add_transform(t, false);}
  // Invert a transform
  void invert();
  // Clear the Transformation
  inline void clear() {
    tran.clear();
    rot.clear();
    tran_flag = false;
    rot_flag = false;
    transform = glm::mat4(1.0);
  }
  // Print the transform to the logs
  inline void print() {
    if (vectors_need_updating) {
      update_vectors();
    }
    if (tran_flag) {
      Poco::Logger::get("Data").debug("{\"Translation\": [%f, %f, %f]}", tran[0], tran[1], tran[2]);
    }
    if (rot_flag) {
      Poco::Logger::get("Data").debug("{\"Rotation\": [%f, %f, %f]}", rot[0], rot[1], rot[2]);
    }
  }
};

#endif  // SRC_MODEL_INCLUDE_TRANSFORM_H_
