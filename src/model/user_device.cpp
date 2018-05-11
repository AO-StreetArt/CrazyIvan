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

// Copy Constructor
UserDevice::UserDevice(const UserDevice &ud) {
  key = ud.get_key();
  port = ud.get_port();
  connection_string = ud.get_connection_string();
  hostname = ud.get_hostname();
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
