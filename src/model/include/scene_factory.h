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

#include "scene_interface.h"
#include "scene.h"

#ifndef SRC_MODEL_INCLUDE_SCENE_FACTORY_H_
#define SRC_MODEL_INCLUDE_SCENE_FACTORY_H_

// SceneFactory allows for creation of SceneInterface instances
class SceneFactory {
 public:
  SceneFactory() {}
  ~SceneFactory() {}
  SceneInterface* build_scene() {return new SceneDocument;}
  SceneInterface* build_scene(protoScene::SceneList_Scene scn_data) \
    {return new SceneDocument(scn_data);}
};

#endif  // SRC_MODEL_INCLUDE_SCENE_FACTORY_H_
