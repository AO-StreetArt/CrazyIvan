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

#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

#ifndef SRC_API_INCLUDE_JSON_SCENE_LIST_H_
#define SRC_API_INCLUDE_JSON_SCENE_LIST_H_

// A single scene message, may include data for multiple scene objects
class JsonSceneList : public SceneList {
  const rapidjson::Value *ms_type_val;
  const rapidjson::Value *err_code_val;
  const rapidjson::Value *err_str_val;
  const rapidjson::Value *tran_id_val;
  const rapidjson::Value *records_val;
  SceneFactory sfactory;

 public:
  // Constructor
  JsonSceneList() : SceneList() {}
  // Constructor accepting Rapidjson Document as input
  // Parse the string and populate the scene object with the information
  JsonSceneList(const rapidjson::Document& d);
  // Destructor
  inline ~JsonSceneList() {}
  // Convert the scene information into a message string
  void to_msg_string(std::string &out_string);
};
#endif  // SRC_API_INCLUDE_JSON_SCENE_LIST_H_
