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

  trans->translate(0, 1.0);
  trans->translate(1, 2.0);
  trans->translate(2, 3.0);
  trans->rotate(0, 45.0);
  trans->rotate(1, 90.0);
  trans->rotate(2, 135.0);

  // Basic Tests
  UserDeviceFactory udfactory;
  std::string udk = "testing";
  UserDeviceInterface *ud = udfactory.build_device(udk, trans);

  assert(ud->has_transform());

  // Getters
  std::string comp_key = "testing";
  assert(ud->get_key() == comp_key);
  assert(ud->get_translation(0) - 1.0 < TOLERANCE);
  assert(ud->get_translation(1) - 2.0 < TOLERANCE);
  assert(ud->get_translation(2) - 3.0 < TOLERANCE);
  assert(ud->get_rotation(0) - 45.0 < TOLERANCE);
  assert(ud->get_rotation(1) - 90.0 < TOLERANCE);
  assert(ud->get_rotation(2) - 135.0 < TOLERANCE);

  shutdown_logging_submodules();

  delete ud;
  delete logging;
  delete factory;

  return 0;
}
