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

#include "include/scene_list.h"
#include "include/scene_interface.h"
#include "include/scene_factory.h"
#include "include/ivan_log.h"
#include "include/ivan_utils.h"
#include "include/Scene.pb.h"

#ifndef SRC_API_INCLUDE_PROTOBUF_SCENE_LIST_H_
#define SRC_API_INCLUDE_PROTOBUF_SCENE_LIST_H_

// A single scene message, may include data for multiple scene objects
class ProtobufSceneList : public SceneList {
  SceneFactory sfactory;

 public:
  // Constructor
  ProtobufSceneList() : SceneList() {}
  // Constructor accepting Protocol Buffer serialized string
  // Here we parse the string and populate the SceneList object with the information
  ProtobufSceneList(protoScene::SceneList buffer);
  // Destructor
  inline ~ProtobufSceneList() {}
  // Convert the scene information into a message string
  void to_msg_string(std::string &out_string);
};

#endif  // SRC_API_INCLUDE_PROTOBUF_SCENE_LIST_H_
