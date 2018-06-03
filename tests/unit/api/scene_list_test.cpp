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

#include "include/scene_list_interface.h"
#include "include/scene_list_factory.h"
#include "include/scene_interface.h"
#include "include/scene_factory.h"
#include "include/transform_interface.h"
#include "include/transform_factory.h"
#include "include/user_device_interface.h"
#include "include/user_device_factory.h"

#include "catch.hpp"

TEST_CASE( "Test Scene List Data Structure", "[unit]" ) {
  // Tolerance
  const double TOLERANCE = 0.001;

  // Basic Setup

  // Setup transforms to test with
  TransformFactory tfactory;
  TransformInterface *trans = tfactory.build_transform();
  TransformInterface *trans2 = tfactory.build_transform();

  trans->translate(0, 1.0);
  trans->translate(1, 2.0);
  trans->translate(2, 3.0);
  trans->rotate(0, 45.0);
  trans->rotate(1, 90.0);
  trans->rotate(2, 135.0);
  trans2->translate(0, 1.0);
  trans2->translate(1, 2.0);
  trans2->translate(2, 3.0);
  trans2->rotate(0, 45.0);
  trans2->rotate(1, 90.0);
  trans2->rotate(2, 135.0);

  // Setup devices to test with
  UserDeviceFactory udfactory;
  std::string udk = "testing";
  UserDeviceInterface *ud = udfactory.build_device(udk, trans);
  UserDeviceInterface *ud2 = udfactory.build_device(udk, trans2);

  // Setup Scenes to test with
  SceneFactory sfactory;
  SceneInterface *scene = sfactory.build_scene();
  SceneInterface *scene2 = sfactory.build_scene();

  std::string sk = "12345";
  std::string sname = "Test Name";
  std::string asset1 = "First Asset";
  std::string asset2 = "Second Asset";
  std::string asset3 = "Third Asset";
  std::string asset4 = "Fourth Asset";
  std::string tag1 = "First Tag";
  std::string tag2 = "Second Tag";
  std::string tag3 = "Third Tag";
  std::string tag4 = "Fourth Tag";
  std::string region = "USA";
  std::string region2 = "Italy";
  scene->set_key(sk);
  scene->set_name(sname);
  scene->set_latitude(100.0);
  scene->set_longitude(120.0);
  scene->set_distance(20.0);
  scene->add_asset(asset1);
  scene->add_asset(asset2);
  scene->add_tag(tag1);
  scene->add_tag(tag2);
  scene->set_region(region);
  scene2->set_key(sk);
  scene2->set_name(sname);
  scene2->set_latitude(100.0);
  scene2->set_longitude(120.0);
  scene2->set_distance(20.0);
  scene2->add_asset(asset3);
  scene2->add_asset(asset4);
  scene2->add_tag(tag3);
  scene2->add_tag(tag4);
  scene2->set_region(region2);

  scene->add_device(ud);
  scene2->add_device(ud2);

  // Basic Tests
  SceneListFactory slfactory;
  SceneListInterface *json_scene_list = slfactory.build_json_scene();

  json_scene_list->set_msg_type(0);
  json_scene_list->set_err_code(110);
  std::string err_message = "Error Message";
  json_scene_list->set_err_msg(err_message);
  std::string tran_id = "12345";
  json_scene_list->set_transaction_id(tran_id);
  json_scene_list->set_num_records(4);
  json_scene_list->add_scene(scene);

  json_scene_list->print();

  // Convert the Scene Lists to messages
  std::string json_msg_str;
  json_scene_list->to_msg_string(json_msg_str);

  // Translate the messages
  rapidjson::Document d;
  d.Parse(json_msg_str.c_str());

  // Build translated Lists
  SceneListInterface *translated_json_list = slfactory.build_scene(d);
  translated_json_list->print();

  // JSON Tests
  REQUIRE(translated_json_list->get_msg_type() == 0);
  REQUIRE(translated_json_list->get_err_code() == 110);
  REQUIRE(translated_json_list->get_transaction_id() == "12345");
  REQUIRE(translated_json_list->get_err() == "Error Message");
  REQUIRE(translated_json_list->get_num_records() == 1);

  REQUIRE(translated_json_list->get_scene(0)->get_key() == "12345");
  REQUIRE(translated_json_list->get_scene(0)->get_name() == "Test Name");
  REQUIRE(translated_json_list->get_scene(0)->get_region() == "USA");
  REQUIRE(translated_json_list->get_scene(0)->get_latitude() - 100.0 < TOLERANCE);
  REQUIRE(translated_json_list->get_scene(0)->get_longitude() - 120.0 < TOLERANCE);
  REQUIRE(translated_json_list->get_scene(0)->get_distance() - 20.0 < TOLERANCE);
  REQUIRE(translated_json_list->get_scene(0)->get_asset(0) == "First Asset");
  REQUIRE(translated_json_list->get_scene(0)->get_asset(1) == "Second Asset");
  REQUIRE(translated_json_list->get_scene(0)->get_tag(0) == "First Tag");
  REQUIRE(translated_json_list->get_scene(0)->get_tag(1) == "Second Tag");

  std::string comp_key = "testing";
  REQUIRE(translated_json_list->get_scene(0)->get_device(0)->get_key() == comp_key);
  REQUIRE(translated_json_list->get_scene(0)->get_device(0)->get_translation(0) - 1.0 < TOLERANCE);
  REQUIRE(translated_json_list->get_scene(0)->get_device(0)->get_translation(1) - 2.0 < TOLERANCE);
  REQUIRE(translated_json_list->get_scene(0)->get_device(0)->get_translation(2) - 3.0 < TOLERANCE);
  REQUIRE(translated_json_list->get_scene(0)->get_device(0)->get_rotation(0) - 45.0 < TOLERANCE);
  REQUIRE(translated_json_list->get_scene(0)->get_device(0)->get_rotation(1) - 90.0 < TOLERANCE);
  REQUIRE(translated_json_list->get_scene(0)->get_device(0)->get_rotation(2) - 135.0 < TOLERANCE);

  delete json_scene_list;
  delete translated_json_list;
}
