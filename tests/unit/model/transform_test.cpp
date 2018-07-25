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

#include <string>
#include <iostream>

#include "include/transform_interface.h"
#include "include/transform_factory.h"

#include "catch.hpp"

TEST_CASE( "Test Transform Data Structure", "[unit]" ) {
  // Tolerance
  const double TOLERANCE = 0.001;

  // Basic Tests
  TransformFactory tfactory;
  TransformInterface *trans = tfactory.build_transform();

  REQUIRE(!(trans->has_translation()));
  REQUIRE(!(trans->has_rotation()));

  trans->translate(0, 1.0);
  trans->translate(1, 2.0);
  trans->translate(2, 3.0);

  REQUIRE(trans->translation(0) - 1.0 < TOLERANCE);
  REQUIRE(trans->translation(1) - 2.0 < TOLERANCE);
  REQUIRE(trans->translation(2) - 3.0 < TOLERANCE);

  trans->rotate(0, 45.0);
  trans->rotate(1, 90.0);
  trans->rotate(2, 135.0);

  REQUIRE(trans->rotation(0) - 45.0 < TOLERANCE);
  REQUIRE(trans->rotation(1) - 90.0 < TOLERANCE);
  REQUIRE(trans->rotation(2) - 135.0 < TOLERANCE);

  trans->invert();

  REQUIRE(trans->translation(0) + 1.0 < TOLERANCE);
  REQUIRE(trans->translation(1) - 1.71554 < TOLERANCE);
  REQUIRE(trans->translation(2) + 2.3627 < TOLERANCE);
  REQUIRE(trans->rotation(0) + 1.83 < TOLERANCE);
  REQUIRE(trans->rotation(1) + 0.403 < TOLERANCE);
  REQUIRE(trans->rotation(2) - 1.0641 < TOLERANCE);

  // Add Transform Tests
  TransformInterface *trans2 = tfactory.build_transform();

  trans2->translate(0, 2.0);
  trans2->translate(1, 2.0);
  trans2->translate(2, 2.0);

  trans2->rotate(0, 60.0);
  trans2->rotate(1, 120.0);
  trans2->rotate(2, 180.0);

  trans->add_transform(trans2);

  REQUIRE(trans->translation(0) - 1.0 < TOLERANCE);
  REQUIRE(trans->translation(1) - 1.7155 < TOLERANCE);
  REQUIRE(trans->translation(2) + 1.0 < TOLERANCE);
  REQUIRE(trans->rotation(0) - 15.0 < TOLERANCE);
  REQUIRE(trans->rotation(1) - 30.0 < TOLERANCE);
  REQUIRE(trans->rotation(2) - 45.0 < TOLERANCE);

  trans2->clear();
  REQUIRE(!(trans2->has_translation()));
  REQUIRE(!(trans2->has_rotation()));

  trans->print();

  delete trans;
  delete trans2;
}
