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

#include "include/base_query_helper.h"
#include "include/scene_interface.h"
#include "include/scene_factory.h"
#include "include/transform_interface.h"
#include "include/transform_factory.h"
#include "include/user_device_interface.h"
#include "include/user_device_factory.h"
#include "include/ivan_log.h"

#include "aossl/logging/include/logging_interface.h"
#include "aossl/logging/include/factory_logging.h"
#include "aossl/neo4j/include/neo4j_interface.h"
#include "aossl/neo4j/include/factory_neo4j.h"

int main() {
  // Initial Setup
  LoggingComponentFactory *factory = new LoggingComponentFactory;
  Neo4jComponentFactory *neo4j_factory = new Neo4jComponentFactory;

  std::string initFileName = "test/log4cpp.properties";
  logging = factory->get_logging_interface(initFileName);

  start_logging_submodules();

  // Basic Setup
  main_logging->info("Starting Query Helper Test");

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
  std::string region = "USA";
  scene->set_key(sk);
  scene->set_name(sname);
  scene->add_asset(asset1);
  scene->add_asset(asset2);
  scene->add_tag(tag1);
  scene->add_tag(tag2);
  scene->set_region(region);
  scene->set_latitude(100.0);
  scene->set_longitude(120.0);
  scene->set_distance(20.0);

  scene->add_device(ud);
  scene->set_transform(scene_trans);

  scene->print();

  // Build a query helper.  We pass in null for the neo4j interface and config
  // manager because we aren't going to call any methods that need these
  // interfaces.
  BaseQueryHelper qh(NULL, neo4j_factory, NULL);

  // Test the query builder methods from the Base Query Helper
  std::string query_string;

  // Build a create query
  qh.generate_scene_crud_query(sk, CREATE_QUERY_TYPE, APPEND, scene, query_string);
  // Validate the query
  main_logging->debug(query_string);
  assert(query_string == "CREATE (scn:Scene {key: {inp_key}, active: {inp_active}, name: {inp_name}, latitude: {inp_lat}, longitude: {inp_long}, region: {inp_region}, assets: {inp_assets}, tags: {inp_tags}}) RETURN scn");

  query_string.clear();

  // Build a get query (by key)
  qh.generate_scene_crud_query(sk, GET_QUERY_TYPE, APPEND, NULL, query_string);
  // Validate the query
  main_logging->debug(query_string);
  assert(query_string == "MATCH (scn:Scene {key: {inp_key}}) RETURN scn");

  query_string.clear();

  // Build an update query
  qh.generate_scene_crud_query(sk, UPDATE_QUERY_TYPE, APPEND, scene, query_string);
  // Validate the query
  main_logging->debug(query_string);
  assert(query_string == "MATCH (scn:Scene {key: {inp_key}}) SET scn.active = {inp_active}, scn.name = {inp_name}, scn.latitude = {inp_lat}, scn.longitude = {inp_long}, scn.region = {inp_region}, scn.assets = coalesce(scn.assets, []) + {inp_assets}, scn.tags = coalesce(scn.tags, []) + {inp_tags} RETURN scn");

  query_string.clear();

  // Build a get query
  std::string blank_key = "";
  qh.generate_scene_crud_query(blank_key, GET_QUERY_TYPE, APPEND, scene, query_string);
  // Validate the query
  main_logging->debug(query_string);
  assert(query_string == "MATCH (scn:Scene {name: {inp_name}, region: {inp_region}}) WHERE {inp_tag} in scn.tags AND ( 12742000 * asin(sqrt(haversin(radians({inp_lat} - scn.latitude)) + cos(radians({inp_lat})) * cos(radians(scn.latitude)) * haversin(radians(scn.longitude - {inp_long}))))) < {inp_distance} RETURN scn");

  query_string.clear();

  // Build an update query
  qh.generate_scene_crud_query(sk, DELETE_QUERY_TYPE, APPEND, NULL, query_string);
  // Validate the query
  main_logging->debug(query_string);
  assert(query_string == "MATCH (scn:Scene {key: {inp_key}}) DETACH DELETE scn RETURN scn");

  // Build & validate lists of query parameters
  std::unordered_map<std::string, Neo4jQueryParameterInterface*> \
    scene_params;
  qh.generate_scene_query_parameters(sk, CREATE_QUERY_TYPE, scene, scene_params);
  assert(scene_params.size() == 8);

  std::unordered_map<std::string, Neo4jQueryParameterInterface*> \
    scene_params2;
  qh.generate_scene_query_parameters(sk, CREATE_QUERY_TYPE, NULL, scene_params2);
  assert(scene_params2.size() == 1);

  for (std::pair<std::string, Neo4jQueryParameterInterface*> element : scene_params) {
    delete element.second;
  }

  for (std::pair<std::string, Neo4jQueryParameterInterface*> element2 : scene_params2) {
    delete element2.second;
  }

  shutdown_logging_submodules();

  delete scene;
  delete logging;
  delete factory;
  delete neo4j_factory;

  return 0;
}
