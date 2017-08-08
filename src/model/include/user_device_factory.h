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

#include "user_device_interface.h"
#include "user_device.h"

#ifndef SRC_MODEL_INCLUDE_USER_DEVICE_FACTORY_H_
#define SRC_MODEL_INCLUDE_USER_DEVICE_FACTORY_H_

// UserDeviceFactory allows for creation of UserDeviceInterface instances
class UserDeviceFactory {
 public:
  UserDeviceFactory() {}
  ~UserDeviceFactory() {}
  UserDeviceInterface* build_device() {return new UserDevice;}
  UserDeviceInterface* build_device(protoScene::SceneList_UserDevice ud_data) \
    {return new UserDevice(ud_data);}
  UserDeviceInterface* build_device(std::string new_key) \
    {return new UserDevice(new_key);}
  UserDeviceInterface* build_device(std::string new_key, TransformInterface *transform) \
    {return new UserDevice(new_key, transform);}
  UserDeviceInterface* build_device(TransformInterface *transform) \
    {return new UserDevice(transform);}
};

# endif  // SRC_MODEL_INCLUDE_USER_DEVICE_FACTORY_H_
