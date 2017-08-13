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

  // Basic Tests
  TransformFactory tfactory;
  TransformInterface *trans = tfactory.build_transform();

  assert(!(trans->has_translation()));
  assert(!(trans->has_rotation()));

  trans->translate(0, 1.0);
  trans->translate(1, 2.0);
  trans->translate(2, 3.0);

  assert(trans->translation(0) - 1.0 < TOLERANCE);
  assert(trans->translation(1) - 2.0 < TOLERANCE);
  assert(trans->translation(2) - 3.0 < TOLERANCE);

  trans->rotate(0, 45.0);
  trans->rotate(1, 90.0);
  trans->rotate(2, 135.0);

  assert(trans->rotation(0) - 45.0 < TOLERANCE);
  assert(trans->rotation(1) - 90.0 < TOLERANCE);
  assert(trans->rotation(2) - 135.0 < TOLERANCE);

  trans->invert();

  assert(trans->translation(0) + 1.0 < TOLERANCE);
  assert(trans->translation(1) + 2.0 < TOLERANCE);
  assert(trans->translation(2) + 3.0 < TOLERANCE);
  assert(trans->rotation(0) + 45.0 < TOLERANCE);
  assert(trans->rotation(1) + 90.0 < TOLERANCE);
  assert(trans->rotation(2) + 135.0 < TOLERANCE);

  // Add Transform Tests
  TransformInterface *trans2 = tfactory.build_transform();

  trans2->translate(0, 2.0);
  trans2->translate(1, 2.0);
  trans2->translate(2, 2.0);

  trans2->rotate(0, 60.0);
  trans2->rotate(1, 120.0);
  trans2->rotate(2, 180.0);

  trans->add_transform(trans2);

  assert(trans->translation(0) - 1.0 < TOLERANCE);
  assert(trans->translation(1) < TOLERANCE);
  assert(trans->translation(2) + 1.0 < TOLERANCE);
  assert(trans->rotation(0) - 15.0 < TOLERANCE);
  assert(trans->rotation(1) - 30.0 < TOLERANCE);
  assert(trans->rotation(2) - 45.0 < TOLERANCE);

  trans2->clear();
  assert(!(trans2->has_translation()));
  assert(!(trans2->has_rotation()));

  trans->print();

  shutdown_logging_submodules();

  delete trans;
  delete trans2;
  delete logging;
  delete factory;

  return 0;
}
