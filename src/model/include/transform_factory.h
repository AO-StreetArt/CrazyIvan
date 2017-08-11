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

#include "transform_interface.h"
#include "transform.h"

#ifndef SRC_MODEL_INCLUDE_TRANSFORM_FACTORY_H_
#define SRC_MODEL_INCLUDE_TRANSFORM_FACTORY_H_

// TransformFactory allows for creation of TransformInterface instances
class TransformFactory {
 public:
  TransformFactory() {}
  ~TransformFactory() {}
  TransformInterface* build_transform() {return new Transform;}
  TransformInterface* \
    build_transform(protoScene::SceneList_Transformation data) \
    {return new Transform(data);}
};

#endif  // SRC_MODEL_INCLUDE_TRANSFORM_FACTORY_H_
