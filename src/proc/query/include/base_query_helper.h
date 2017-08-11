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

#include "include/ivan_log.h"
#include "include/ivan_utils.h"

#include "include/transform_interface.h"
#include "include/transform_factory.h"
#include "include/user_device_interface.h"
#include "include/user_device_factory.h"
#include "include/scene_interface.h"
#include "include/scene_factory.h"

#include "include/scene_list_interface.h"
#include "include/scene_list_factory.h"

#include "aossl/neo4j/include/neo4j_interface.h"
#include "aossl/neo4j/include/factory_neo4j.h"

#ifndef SRC_PROC_QUERY_INCLUDE_BASE_QUERY_HELPER_H_
#define SRC_PROC_QUERY_INCLUDE_BASE_QUERY_HELPER_H_

// The class contains helper methods for working with
// Scene Data in Neo4j
class BaseQueryHelper {
  ConfigurationManager *config = NULL;
  Neo4jInterface *n = NULL;
  Neo4jComponentFactory *neo_factory = NULL;
  SceneFactory sfactory;
  SceneListFactory slfactory;
  TransformFactory tfactory;
  UserDeviceFactory udfactory;
 public:
  inline BaseQueryHelper(Neo4jInterface *neo, Neo4jComponentFactory *nf, ConfigurationManager *con) {
    n = neo;
    neo_factory = nf;
    config = con;
  }
  virtual ~BaseQueryHelper() {}

  // Create new transformations
  inline TransformInterface* create_transform() {return tfactory.build_transform();}
  inline TransformInterface* create_transform(protoScene::SceneList_Transformation data) \
   {return tfactory.build_transform(data);}

  // Create new User Devices
  inline UserDeviceInterface* create_device() {return udfactory.build_device();}
  inline UserDeviceInterface* create_device(protoScene::SceneList_UserDevice ud_data) \
   {return udfactory.build_device(ud_data);}
  inline UserDeviceInterface* create_device(std::string inp_key, TransformInterface *t) \
    {return udfactory.build_device(inp_key, t);}
  inline UserDeviceInterface* create_device(std::string inp_key) \
    {return udfactory.build_device(inp_key);}

  // Create new Scenes
  inline SceneInterface* create_scene() {return sfactory.build_scene();}
  inline SceneInterface* create_scene(protoScene::SceneList_Scene scn_data) \
    {return sfactory.build_scene(scn_data);}

  // Create new SceneLists
  inline SceneListInterface* create_json_scene() \
    {return slfactory.build_json_scene();}
  inline SceneListInterface* create_protobuf_scene() \
    {return slfactory.build_protobuf_scene();}
  inline SceneListInterface* create_scene(const rapidjson::Document& d) \
    {return slfactory.build_scene(d);}
  inline SceneListInterface* create_scene(protoScene::SceneList buffer) \
    {return slfactory.build_scene(buffer);}

  // Get other basic pointers
  ConfigurationManager* get_config_manager() {return config;}
  Neo4jInterface* get_neo4j_interface() {return n;}
  Neo4jComponentFactory* get_neo4j_factory() {return neo_factory;}
};

#endif  // SRC_PROC_QUERY_INCLUDE_BASE_QUERY_HELPER_H_
