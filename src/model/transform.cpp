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

// Invert a transform
void Transform::invert() {
  tran[0] = (-1.0) * tran[0];
  tran[1] = (-1.0) * tran[1];
  tran[2] = (-1.0) * tran[2];
  rot[0] = (-1.0) * rot[0];
  rot[1] = (-1.0) * rot[1];
  rot[2] = (-1.0) * rot[2];
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
}

// Create a transform from a Protocol Buffer Transformation
Transform::Transform(protoScene::SceneList_Transformation data) {
  obj_logging->debug("Converting Transform Data from Protocol Buffer");

  tran_flag = false;
  rot_flag = false;
  tran.push_back(0.0);
  tran.push_back(0.0);
  tran.push_back(0.0);
  rot.push_back(0.0);
  rot.push_back(0.0);
  rot.push_back(0.0);
  if (data.has_translation()) {
    protoScene::SceneList_Vertex3 ptrans = data.translation();
    tran[0] = ptrans.x();
    tran[1] = ptrans.y();
    tran[2] = ptrans.z();
    tran_flag = true;
    obj_logging->debug("Translation added");
  }
  if (data.has_rotation()) {
    protoScene::SceneList_Vertex3 prot = data.rotation();
    rot[0] = prot.x();
    rot[1] = prot.y();
    rot[2] = prot.z();
    rot_flag = true;
    obj_logging->debug("Rotation added");
  }
}

// Add a Transform together
void Transform::add_transform(TransformInterface *t, bool inverted) {
  if (t->has_translation()) {
    if (inverted) {
      tran[0] = tran[0] - t->translation(0);
      tran[1] = tran[1] - t->translation(1);
      tran[2] = tran[2] - t->translation(2);
    } else {
      tran[0] = tran[0] + t->translation(0);
      tran[1] = tran[1] + t->translation(1);
      tran[2] = tran[2] + t->translation(2);
    }
    tran_flag = true;
  }
  if (t->has_rotation()) {
    if (inverted) {
      rot[0] = rot[0] - t->rotation(0);
      rot[1] = rot[1] - t->rotation(1);
      rot[2] = rot[2] - t->rotation(2);
    } else {
      rot[0] = rot[0] + t->rotation(0);
      rot[1] = rot[1] + t->rotation(1);
      rot[2] = rot[2] + t->rotation(2);
    }
    rot_flag = true;
  }
}
