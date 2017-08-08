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

#include "scene_list_interface.h"
#include "json_scene_list.h"
#include "protobuf_scene_list.h"

#ifndef SRC_API_INCLUDE_SCENE_LIST_FACTORY_H_
#define SRC_API_INCLUDE_SCENE_LIST_FACTORY_H_

// SceneFactory allows for creation of SceneInterface instances
class SceneListFactory {
 public:
  SceneListFactory() {}
  ~SceneListFactory() {}
  SceneListInterface* build_json_scene() {return new JsonSceneList;}
  SceneListInterface* build_protobuf_scene() {return new ProtobufSceneList;}
  SceneListInterface* build_scene(const rapidjson::Document& d) \
    {return new JsonSceneList(d);}
  SceneListInterface* build_scene(protoScene::SceneList buffer) \
    {return new ProtobufSceneList(buffer);}
};

# endif  // SRC_API_INCLUDE_SCENE_LIST_FACTORY_H_
