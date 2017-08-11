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

#include "include/scene_query_helper.h"

bool SceneQueryHelper::scene_exists(std::string inp_key) {
  // Determine if the scene exists
  processor_logging->debug("Determine if the scene exists");
  ResultsIteratorInterface *results = NULL;
  ResultTreeInterface *tree = NULL;
  DbObjectInterface *obj = NULL;
  Neo4jQueryParameterInterface *query_param = NULL;
  // Create the query string
  std::string query_string =
    "MATCH (scn:Scene {key: {inp_key}})"
    " RETURN scn";

  // Set up the query parameters for query
  std::unordered_map<std::string, Neo4jQueryParameterInterface*> query_params;
  query_param = \
    BaseQueryHelper::get_neo4j_factory()->get_neo4j_query_parameter(inp_key);
  processor_logging->debug("Key:");
  processor_logging->debug(inp_key);
  query_params.emplace("inp_key", query_param);

  // Execute the query
  bool ret_val = false;
  bool has_exception = false;
  std::string exc_string = "";
  try {
    results = BaseQueryHelper::get_neo4j_interface()->execute(query_string, \
      query_params);
  }
  catch (std::exception& e) {
    processor_logging->error("Error running Query:");
    processor_logging->error(query_string);
    processor_logging->error(e.what());
    has_exception = true;
    std::string e_string(e.what());
    exc_string = e_string;
  }

  if (!results) {
    processor_logging->debug("No Scenes found for the given key");
  } else {
    tree = results->next();
    if (tree) {
      obj = tree->get(0);
      if (obj->is_node()) ret_val = true;
    }
  }
  if (obj) delete obj;
  if (tree) delete tree;
  if (results) delete results;
  if (query_param) delete query_param;
  if (has_exception) throw QueryException(exc_string);
  return ret_val;
}

// Get the link between two scenes
// Return:
// --0: If no link is found
// --1: If a forward link is found
// --2: If a backward link is found
int SceneQueryHelper::get_scene_link(std::string scene1_key, \
  std::string scene2_key) {
  // Query the DB for an existing connection between the two scenes
  processor_logging->debug("Querying DB for connections between scenes");
  ResultsIteratorInterface *results = NULL;
  ResultTreeInterface *tree = NULL;
  DbObjectInterface* obj = NULL;
  DbMapInterface* map = NULL;
  Neo4jQueryParameterInterface* skey1_param = NULL;
  Neo4jQueryParameterInterface* skey2_param = NULL;

  // Create the query string
  std::string query_string =
    "MATCH (scn:Scene)-[trans:TRANSFORM]->(scn2:Scene)"
    " WHERE (scn.key = {inp_key1} AND scn2.key = {inp_key2})"
    " OR (scn.key = {inp_key2} AND scn2.key = {inp_key1})"
    " RETURN trans";

  // Set up the query parameters for query
  std::unordered_map<std::string, Neo4jQueryParameterInterface*> q_params;

  // Insert the first scene key into the query list
  skey1_param = \
    BaseQueryHelper::get_neo4j_factory()->get_neo4j_query_parameter(scene1_key);
  processor_logging->debug("Scene 1 Key:");
  processor_logging->debug(scene1_key);
  q_params.emplace("inp_key1", skey1_param);

  // Insert the second scene key into the query list
  skey2_param = \
    BaseQueryHelper::get_neo4j_factory()->get_neo4j_query_parameter(scene2_key);
  processor_logging->debug("Scene 2 Key:");
  processor_logging->debug(scene2_key);
  q_params.emplace("inp_key2", skey2_param);

  // Execute the query
  int ret_val = 0;
  try {
    results = \
      BaseQueryHelper::get_neo4j_interface()->execute(query_string, q_params);
  }
  catch (std::exception& e) {
    processor_logging->error("Error running Query:");
    processor_logging->error(query_string);
    processor_logging->error(e.what());
  }
  if (results) {
    // Find if the first result is forward or backward
    tree = results->next();
    if (!tree) {
      processor_logging->debug("No values found in result tree");
    } else {
      obj = tree->get(0);
      if (!(obj->is_edge())) {
        processor_logging->debug("Non-Edge value returned from query");
      } else {
        map = obj->properties();
        if (map->element_exists("key")) {
          if (scene1_key == map->get_string_element("key")) {
            ret_val = 1;
          } else if (scene2_key == map->get_string_element("key")) {
            ret_val = 2;
          }
        }
      }
    }
  }

  if (map) delete map;
  if (obj) delete obj;
  if (tree) delete tree;
  if (results) delete results;
  if (skey1_param) delete skey1_param;
  if (skey2_param) delete skey2_param;

  return ret_val;
}

// Create the scene-scene link
void SceneQueryHelper::create_scene_link(std::string s1_key, \
  std::string s2_key, TransformInterface *new_trans) {
  processor_logging->debug("Creating Device Registration link");
  ResultsIteratorInterface *results = NULL;
  ResultTreeInterface *tree = NULL;
  DbObjectInterface* obj = NULL;
  Neo4jQueryParameterInterface* s1key_param = NULL;
  Neo4jQueryParameterInterface* s2key_param = NULL;
  Neo4jQueryParameterInterface* locx_param = NULL;
  Neo4jQueryParameterInterface* locy_param = NULL;
  Neo4jQueryParameterInterface* locz_param = NULL;
  Neo4jQueryParameterInterface* rotx_param = NULL;
  Neo4jQueryParameterInterface* roty_param = NULL;
  Neo4jQueryParameterInterface* rotz_param = NULL;
  // Create the query string
  std::string udq_string =
    "MATCH (scn:Scene {key: {inp_key1}}), (scn2:Scene {key: {inp_key2}}) "
    "CREATE (scn)-[trans:TRANSFORM {translation_x: {loc_x}, "
      "translation_y: {loc_y}, translation_z: {loc_z}, rotation_x: {rot_x}, "
      "rotation_y: {rot_y}, rotation_z: {rot_z}}]->(scn2) "
    "RETURN scn, trans, scn2";

  // Set up the query parameters for query
  std::unordered_map<std::string, Neo4jQueryParameterInterface*> q_params;

  // Insert the scene key into the query list
  s1key_param = \
    BaseQueryHelper::get_neo4j_factory()->get_neo4j_query_parameter(s1_key);
  processor_logging->debug("Scene 1 Key:");
  processor_logging->debug(s1_key);
  q_params.emplace("inp_key1", s1key_param);

  // Insert the device key into the query list
  s2key_param = \
    BaseQueryHelper::get_neo4j_factory()->get_neo4j_query_parameter(s2_key);
  processor_logging->debug("Scene 2 Key:");
  processor_logging->debug(s2_key);
  q_params.emplace("inp_key2", s2key_param);

  // Insert translation
  locx_param = BaseQueryHelper::get_neo4j_factory()->\
    get_neo4j_query_parameter(new_trans->translation(0));
  locy_param = BaseQueryHelper::get_neo4j_factory()->\
    get_neo4j_query_parameter(new_trans->translation(1));
  locz_param = BaseQueryHelper::get_neo4j_factory()->\
    get_neo4j_query_parameter(new_trans->translation(2));
  q_params.emplace("loc_x", locx_param);
  q_params.emplace("loc_y", locy_param);
  q_params.emplace("loc_z", locz_param);

  // Insert rotation
  rotx_param = BaseQueryHelper::get_neo4j_factory()->\
    get_neo4j_query_parameter(new_trans->rotation(0));
  roty_param = BaseQueryHelper::get_neo4j_factory()->\
    get_neo4j_query_parameter(new_trans->rotation(1));
  rotz_param = BaseQueryHelper::get_neo4j_factory()->\
    get_neo4j_query_parameter(new_trans->rotation(2));
  q_params.emplace("rot_x", rotx_param);
  q_params.emplace("rot_y", roty_param);
  q_params.emplace("rot_z", rotz_param);

  // Execute the query
  bool has_exception = false;
  std::string exc_string = "";
  try {
    results = \
      BaseQueryHelper::get_neo4j_interface()->execute(udq_string, q_params);
  }
  catch (std::exception& e) {
    processor_logging->error("Error running Query:");
    processor_logging->error(udq_string);
    processor_logging->error(e.what());
    has_exception = true;
    std::string e_string(e.what());
    exc_string = e_string;
  }

  if (!results) {
    processor_logging->error("No Links created");
    std::string exc_str = "No Links created: ";
    exc_string = exc_str + udq_string;
    has_exception = true;
  } else {
    tree = results->next();
    if (!tree) {
      processor_logging->error("Query Returned no result tree");
      std::string exc_str = "Query Returned no values: ";
      exc_string = exc_str + udq_string;
      has_exception = true;
    } else {
      obj = tree->get(0);
      if (!(obj->is_node())) {
        processor_logging->debug("Query Returned no values");
        std::string exc_str = "Query Returned no values: ";
        exc_string = exc_str + udq_string;
        has_exception = true;
      }
    }
  }
  if (obj) delete obj;
  if (tree) delete tree;
  if (results) delete results;
  if (s1key_param) delete s1key_param;
  if (s2key_param) delete s2key_param;
  if (locx_param) delete locx_param;
  if (locy_param) delete locy_param;
  if (locz_param) delete locz_param;
  if (rotx_param) delete rotx_param;
  if (roty_param) delete roty_param;
  if (rotz_param) delete rotz_param;
  if (has_exception) throw QueryException(exc_string);
}

// Update the scene-scene link
void SceneQueryHelper::update_scene_link(std::string s1_key, \
  std::string s2_key, TransformInterface *new_trans) {
  processor_logging->debug("Updating Scene link");
  ResultsIteratorInterface *results = NULL;
  ResultTreeInterface *tree = NULL;
  DbObjectInterface* obj = NULL;
  Neo4jQueryParameterInterface* s1key_param = NULL;
  Neo4jQueryParameterInterface* s2key_param = NULL;
  Neo4jQueryParameterInterface* locx_param = NULL;
  Neo4jQueryParameterInterface* locy_param = NULL;
  Neo4jQueryParameterInterface* locz_param = NULL;
  Neo4jQueryParameterInterface* rotx_param = NULL;
  Neo4jQueryParameterInterface* roty_param = NULL;
  Neo4jQueryParameterInterface* rotz_param = NULL;
  // Create the query string
  std::string udq_string =
    "MATCH (scn:Scene {key: {inp_key1}})-[trans:TRANSFORM]->"
      "(scn2:Scene {key: {inp_key2}}) "
    "SET trans.translation_x = {loc_x}, trans.translation_y = {loc_y}, "
      "trans.translation_z = {loc_z}, trans.rotation_x: {rot_x}, "
      "trans.rotation_y: {rot_y}, trans.rotation_z: {rot_z} "
    "RETURN scn, trans, scn2";

  // Set up the query parameters for query
  std::unordered_map<std::string, Neo4jQueryParameterInterface*> q_params;

  // Insert the scene key into the query list
  s1key_param = \
    BaseQueryHelper::get_neo4j_factory()->get_neo4j_query_parameter(s1_key);
  processor_logging->debug("Scene 1 Key:");
  processor_logging->debug(s1_key);
  q_params.emplace("inp_key1", s1key_param);

  // Insert the device key into the query list
  s2key_param = \
    BaseQueryHelper::get_neo4j_factory()->get_neo4j_query_parameter(s2_key);
  processor_logging->debug("Scene 2 Key:");
  processor_logging->debug(s2_key);
  q_params.emplace("inp_key2", s2key_param);

  // Insert translation
  locx_param = BaseQueryHelper::get_neo4j_factory()->\
    get_neo4j_query_parameter(new_trans->translation(0));
  locy_param = BaseQueryHelper::get_neo4j_factory()->\
    get_neo4j_query_parameter(new_trans->translation(1));
  locz_param = BaseQueryHelper::get_neo4j_factory()->\
    get_neo4j_query_parameter(new_trans->translation(2));
  q_params.emplace("loc_x", locx_param);
  q_params.emplace("loc_y", locy_param);
  q_params.emplace("loc_z", locz_param);

  // Insert rotation
  rotx_param = BaseQueryHelper::get_neo4j_factory()->\
    get_neo4j_query_parameter(new_trans->rotation(0));
  roty_param = BaseQueryHelper::get_neo4j_factory()->\
    get_neo4j_query_parameter(new_trans->rotation(1));
  rotz_param = BaseQueryHelper::get_neo4j_factory()->\
    get_neo4j_query_parameter(new_trans->rotation(2));
  q_params.emplace("rot_x", rotx_param);
  q_params.emplace("rot_y", roty_param);
  q_params.emplace("rot_z", rotz_param);

  // Execute the query
  bool has_exception = false;
  std::string exc_string = "";
  try {
    results = \
      BaseQueryHelper::get_neo4j_interface()->execute(udq_string, q_params);
  }
  catch (std::exception& e) {
    processor_logging->error("Error running Query:");
    processor_logging->error(udq_string);
    processor_logging->error(e.what());
    has_exception = true;
    std::string e_string(e.what());
    exc_string = e_string;
  }

  if (!results) {
    processor_logging->error("No Links created");
    has_exception = true;
    std::string exc_str = "No Links created: ";
    exc_string = exc_str + udq_string;
  } else {
    tree = results->next();
    if (!tree) {
      processor_logging->debug("Query Returned no result tree");
      has_exception = true;
      std::string exc_str = "Query Returned no result tree: ";
      exc_string = exc_str + udq_string;
    } else {
      obj = tree->get(0);
      if (!(obj->is_node())) {
        processor_logging->debug("Query Returned no values");
        has_exception = true;
        std::string exc_str = "Query Returned no values: ";
        exc_string = exc_str + udq_string;
      }
    }
  }
  if (obj) delete obj;
  if (tree) delete tree;
  if (results) delete results;
  if (s1key_param) delete s1key_param;
  if (s2key_param) delete s2key_param;
  if (locx_param) delete locx_param;
  if (locy_param) delete locy_param;
  if (locz_param) delete locz_param;
  if (rotx_param) delete rotx_param;
  if (roty_param) delete roty_param;
  if (rotz_param) delete rotz_param;
  if (has_exception) throw QueryException(exc_string);
}
