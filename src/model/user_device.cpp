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

#include "include/user_device.h"

// Create a new user device object from a protocol buffer user device object
UserDevice::UserDevice(protoScene::SceneList_UserDevice ud_data) {
  obj_logging->debug("Converting User Device Data from Protocol Buffer");

  if (ud_data.has_key()) {
    key = ud_data.key();
    obj_logging->debug(key);
  }
  if (ud_data.has_transform()) {
    trans = new Transform (ud_data.transform());
    trns_flag = true;
    obj_logging->debug("Device Transform added");
  }
}

// Copy Constructor
UserDevice::UserDevice(const UserDevice &ud) {
  key = ud.get_key();
  trns_flag = false;
  if (ud.has_transform()) {
    trns_flag = true;
    trans = tfactory.build_transform();
    for (int i = 0; i < 3; i++) {
      if (ud.get_transform()->has_translation()) {
        trans->translate(i, ud.get_translation(i));
      }
      if (ud.get_transform()->has_rotation()) {
        trans->rotate(i, ud.get_rotation(i));
      }
    }
  }
}
