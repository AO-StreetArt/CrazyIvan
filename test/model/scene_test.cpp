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

#include <assert.h>
#include <string>
#include <iostream>

#include "include/scene_interface.h"
#include "include/scene_factory.h"
#include "include/transform_interface.h"
#include "include/transform_factory.h"
#include "include/user_device_interface.h"
#include "include/user_device_factory.h"
#include "include/ivan_log.h"

#include "aossl/logging/include/logging_interface.h"
#include "aossl/logging/include/factory_logging.h"

int main() {
  // Tolerance
  const double TOLERANCE = 0.001;
  // Initial Setup
  LoggingComponentFactory *factory = new LoggingComponentFactory;

  std::string initFileName = "log4cpp.properties";
  logging = factory->get_logging_interface(initFileName);

  start_logging_submodules();

  // Basic Setup

  // Setup a transform to test with
  TransformFactory tfactory;
  TransformInterface *trans = tfactory.build_transform();
  TransformInterface *scene_trans = tfactory.build_transform();

  trans->translate(0, 1.0);
  trans->translate(1, 2.0);
  trans->translate(2, 3.0);
  trans->rotate(0, 45.0);
  trans->rotate(1, 90.0);
  trans->rotate(2, 135.0);

  scene_trans->translate(0, 1.0);
  scene_trans->translate(1, 1.0);
  scene_trans->translate(2, 1.0);
  scene_trans->rotate(0, 10.0);
  scene_trans->rotate(1, 20.0);
  scene_trans->rotate(2, 30.0);

  // Setup a device to test with
  UserDeviceFactory udfactory;
  std::string udk = "testing";
  UserDeviceInterface *ud = udfactory.build_device(udk, trans);

  // Basic Tests
  SceneFactory sfactory;
  SceneInterface *scene = sfactory.build_scene();

  std::string sk = "12345";
  std::string sname = "Test Name";
  std::string asset1 = "First Asset";
  std::string asset2 = "Second Asset";
  std::string tag1 = "First Tag";
  std::string tag2 = "Second Tag";
  scene->set_key(sk);
  scene->set_name(sname);
  scene->add_asset(asset1);
  scene->add_asset(asset2);
  scene->add_tag(tag1);
  scene->add_tag(tag2);
  scene->set_latitude(100.0);
  scene->set_longitude(120.0);
  scene->set_distance(20.0);

  scene->add_device(ud);
  scene->set_transform(scene_trans);

  assert(scene->num_devices() == 1);
  assert(scene->get_key() == "12345");
  assert(scene->get_name() == "Test Name");
  assert(scene->get_asset(0) == "First Asset");
  assert(scene->get_asset(1) == "Second Asset");
  assert(scene->get_tag(0) == "First Tag");
  assert(scene->get_tag(1) == "Second Tag");
  assert(scene->get_latitude() == 100.0);
  assert(scene->get_longitude() == 120.0);
  assert(scene->get_distance() == 20.0);
  assert(scene->has_transform());
  assert(scene->get_device(0)->get_translation(0) - 1.0 < TOLERANCE);
  assert(scene->get_device(0)->get_translation(1) - 2.0 < TOLERANCE);
  assert(scene->get_device(0)->get_translation(2) - 3.0 < TOLERANCE);
  assert(scene->get_device(0)->get_rotation(0) - 45.0 < TOLERANCE);
  assert(scene->get_device(0)->get_rotation(0) - 90.0 < TOLERANCE);
  assert(scene->get_device(0)->get_rotation(0) - 135.0 < TOLERANCE);
  assert(scene->get_scene_transform()->translation(0) - 1.0 < TOLERANCE);
  assert(scene->get_scene_transform()->translation(1) - 1.0 < TOLERANCE);
  assert(scene->get_scene_transform()->translation(2) - 1.0 < TOLERANCE);
  assert(scene->get_scene_transform()->rotation(0) - 10.0 < TOLERANCE);
  assert(scene->get_scene_transform()->rotation(1) - 20.0 < TOLERANCE);
  assert(scene->get_scene_transform()->rotation(2) - 30.0 < TOLERANCE);

  scene->print();

  shutdown_logging_submodules();

  delete scene;
  delete logging;
  delete factory;

  return 0;
}
