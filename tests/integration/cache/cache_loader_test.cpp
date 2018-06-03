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

#include "include/device_cache.h"
#include "include/cache_loader.h"

#include "neocpp/connection/interface/neo4j_interface.h"
#include "neocpp/connection/impl/libneo4j_factory.h"

#include "catch.hpp"

TEST_CASE( "Test Cache Loader", "[integration]" ) {
  // Setup test objects
  Neocpp::LibNeo4jFactory neo_factory;
  Neocpp::Neo4jInterface *neo = neo_factory.get_neo4j_interface("neo4j://localhost:7687");
  Neocpp::ResultsIteratorInterface *results1 = NULL;
  Neocpp::ResultsIteratorInterface *results2 = NULL;
  Neocpp::ResultsIteratorInterface *results3 = NULL;
  DeviceCache cache;
  DeviceCacheLoader loader(&cache, neo);

  // Setup base data in the DB
  std::string scene_delete_query = "MATCH (scn:Scene {key: \"test\"}) DETACH DELETE scn";
  std::string scene_create_query = "CREATE (scn:Scene {key: \"test\","
      " scn.name = \"test\", scn.latitude = 100.0, scn.longitude = 100.0,"
      " scn.region = \"testRegion\", scn.assets = [\"asset\"],"
      " tags: [\"tag\"] RETURN scn";
  std::string device_register_query = "MATCH (scn:Scene {key: \"test\"})"
      " CREATE (scn)-[trans:TRANSFORM {translation_x: 0.0, "
      "translation_y: 1.0, translation_z: 0.0, rotation_x: 0.0, "
      "rotation_y: 0.0, rotation_z: 0.0]->(ud:UserDevice "
      "{key: \"testUd\", connection_string: \"\", "
      "hostname: \"testHost\", port: 5555) ";
  try {
    results1 = neo->execute(scene_delete_query);
    results2 = neo->execute(scene_create_query);
    results3 = neo->execute(device_register_query);
  } catch (std::exception& e) {
    std::cout << "Error running Neo4j Query: " << e.what() << std::endl;
    if (results1) delete results1;
    if (results2) delete results2;
    if (results3) delete results3;
    delete neo;
    REQUIRE(false);
  }


  // Load the scene into the cache
  std::string scene_name = "test";
  loader.load_scene(scene_name);
  // Validate the cache entries
  std::vector<std::pair<std::string, int>> device_list = cache.get_devices(scene_name);
  if (results1) delete results1;
  if (results2) delete results2;
  if (results3) delete results3;
  delete neo;
  REQUIRE(device_list.size() == 1);
  REQUIRE(device_list[0].first == "testHost");
  REQUIRE(device_list[0].second == 5555);
}
