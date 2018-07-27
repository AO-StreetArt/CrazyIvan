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

#include "include/transform.h"

void Transform::update_vectors() {
  // Convert the current transform matrix into a
  //  translation vector and euler rotation vector
  vectors_need_updating = false;
  // Pull Rotation Vector
  glm::extractEulerAngleXYZ(transform, rot[0], rot[1], rot[2]);
}

// Invert this transform
void Transform::invert() {
  // Invert translation
  for (int i = 0; i < 3; i++) {
    tran[i] = -1*tran[i];
  }
  // Invert rotation
  vectors_need_updating = true;
  glm::mat4 new_transform = glm::inverse(transform);
  transform = new_transform;
}

// Create a transform with default values
Transform::Transform() {
  tran_flag = false;
  rot_flag = false;
  tran.push_back(0.0);
  tran.push_back(0.0);
  tran.push_back(0.0);
  rot.push_back(0.0);
  rot.push_back(0.0);
  rot.push_back(0.0);
  transform = glm::mat4(1.0);
}

// Add a Transform together
void Transform::add_transform(TransformInterface *t, bool inverted) {
  // Apply the provided translation
  for (int i = 0; i < 3; i++) {
    if (inverted) {
      tran[i] = tran[i] - t->translation(i);
    } else {
      tran[i] = tran[i] + t->translation(i);
    }
  }

  // Apply the provided rotation by RHS matrix multiplication
  glm::mat4 new_transform;
  if (inverted) {
    new_transform = glm::inverse(t->get_transform_vector()) * transform;
  } else {
    new_transform = t->get_transform_vector() * transform;
  }
  transform = new_transform;

  // Update our internal flags
  if (t->has_translation()) {
    tran_flag = true;
  }
  if (t->has_rotation()) {
    rot_flag = true;
  }
  vectors_need_updating = true;
}
