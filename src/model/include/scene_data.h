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

#include "user_device_interface.h"
#include "user_device_factory.h"
#include "transform_interface.h"
#include "transform_factory.h"

#ifndef SRC_MODEL_INCLUDE_SCENE_DATA_H_
#define SRC_MODEL_INCLUDE_SCENE_DATA_H_

// Stores the data for a single scene
class SceneData {
  double latitude = 0.0;
  double longitude = 0.0;
  std::vector<UserDeviceInterface*> devices;
  UserDeviceFactory udfactory;
  TransformFactory tfactory;

 public:
  inline TransformInterface* create_transform() {return tfactory.build_transform();}
  inline TransformInterface* create_transform(protoScene::SceneList_Transformation data) \
    {return tfactory.build_transform(data);}
  inline UserDeviceInterface* create_device() {return udfactory.build_device();}
  inline UserDeviceInterface* create_device(protoScene::SceneList_UserDevice ud_data) \
    {return udfactory.build_device(ud_data);}
    inline UserDeviceInterface* create_device(std::string inp_key, TransformInterface *t) \
      {return udfactory.build_device(inp_key, t);}
  // Constructors
  SceneData() {}
  inline SceneData(const SceneData& sd) {
    latitude = sd.get_latitude();
    longitude = sd.get_longitude();
    for (int j = 0; j < sd.num_devices(); j++) {
      TransformInterface *new_tran = create_transform();
      for (int k = 0; k < 3; k++) {
        if (sd.get_device(j)->has_transform()) {
          new_tran->translate(k, sd.get_device(j)->get_translation(k));
          new_tran->rotate(k, sd.get_device(j)->get_rotation(k));
        }
      }
      UserDeviceInterface *new_dev = \
        create_device(sd.get_device(j)->get_key(), new_tran);
      devices.push_back(new_dev);
    }
  }

  // Destructor
  virtual inline ~SceneData() {
    for (unsigned int i = 0; i < devices.size(); i++) {
      if (devices[i]) delete devices[i];
    }
  }

  // Latitude
  void set_latitude(double new_lat) {latitude = new_lat;}
  double get_latitude() const {return latitude;}

  // Longitude
  void set_longitude(double new_long) {longitude = new_long;}
  double get_longitude() const {return longitude;}

  // Device List
  void add_device(UserDeviceInterface *d) {devices.push_back(d);}
  int num_devices() const {return devices.size();}
  UserDeviceInterface* get_device(int index) const {return devices[index];}
  std::vector<UserDeviceInterface*> get_devices() const {return devices;}
};

#endif  // SRC_MODEL_INCLUDE_SCENE_DATA_H_
