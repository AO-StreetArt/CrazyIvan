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
#include <cmath>

#include "include/transform_interface.h"
#include "include/transform_factory.h"

#include "catch.hpp"

void print_transform(TransformInterface *trans) {
  std::cout << "Translation: [" << trans->translation(0) << \
      "," << trans->translation(1) << "," << trans->translation(2) << \
      "], Rotation: [" << trans->rotation(0) << "," << trans->rotation(1) << \
      "," << trans->rotation(2) << "]" << std::endl;
}

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

  std::cout <<"Initial Translation:" << std::endl;
  print_transform(trans);

  REQUIRE(std::abs(trans->translation(0) - 1.0) < TOLERANCE);
  REQUIRE(std::abs(trans->translation(1) - 2.0) < TOLERANCE);
  REQUIRE(std::abs(trans->translation(2) - 3.0) < TOLERANCE);

  trans->rotate(0, 0.79);
  trans->rotate(1, 1.57);
  trans->rotate(2, 2.36);

  std::cout <<"Initial Rotation:" << std::endl;
  print_transform(trans);

  REQUIRE(std::abs(trans->rotation(0) - 0.79) < TOLERANCE);
  REQUIRE(std::abs(trans->rotation(1) - 1.57) < TOLERANCE);
  REQUIRE(std::abs(trans->rotation(2) - 2.36) < TOLERANCE);

  std::cout <<"Initial Transform:" << std::endl;
  print_transform(trans);
  trans->invert();
  std::cout << "Inverted Transform:" << std::endl;
  print_transform(trans);

  REQUIRE(std::abs(trans->translation(0) + 1.0) < TOLERANCE);
  REQUIRE(std::abs(trans->translation(1) + 2.0) < TOLERANCE);
  REQUIRE(std::abs(trans->translation(2) + 3.0) < TOLERANCE);
  REQUIRE(std::abs(trans->rotation(0) - 1.504) < TOLERANCE);
  REQUIRE(std::abs(trans->rotation(1) - 1.562) < TOLERANCE);
  REQUIRE(std::abs(trans->rotation(2) - 1.637) < TOLERANCE);

  // Add Transform Tests
  TransformInterface *trans2 = tfactory.build_transform();

  trans2->translate(0, 2.0);
  trans2->translate(1, 2.0);
  trans2->translate(2, 2.0);

  trans2->rotate(0, 0.79);
  trans2->rotate(1, 1.57);
  trans2->rotate(2, 2.36);

  trans->add_transform(trans2);

  std::cout << "Combined Transform:" << std::endl;
  print_transform(trans);

  REQUIRE(std::abs(trans->translation(0) - 1.0) < TOLERANCE);
  REQUIRE(std::abs(trans->translation(1)) < TOLERANCE);
  REQUIRE(std::abs(trans->translation(2) + 1.0) < TOLERANCE);
  REQUIRE(std::abs(trans->rotation(0)) < TOLERANCE);
  REQUIRE(std::abs(trans->rotation(1)) < TOLERANCE);
  REQUIRE(std::abs(trans->rotation(2)) < TOLERANCE);

  trans2->clear();
  REQUIRE(!(trans2->has_translation()));
  REQUIRE(!(trans2->has_rotation()));

  trans->print();

  delete trans;
  delete trans2;
}
