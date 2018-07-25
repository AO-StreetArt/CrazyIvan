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

#include "include/algorithm_query_helper.h"

// Use a Device registered to multiple scenes to create Scene-Scene
// Transform Links in the DB
// As an input, we expect a set of Scenes and User Device pairs
// which are then processed to generate the Scene-Scene links
void AlgorithmQueryHelper::process_UDUD_transformation(\
  SceneListInterface *registered_scenes, SceneListInterface *obj_msg) {
  BaseQueryHelper::logger().debug("Processing Scene-Scene Links");

  // Get the number of scenes
  int num_scenes = registered_scenes->num_scenes();
  int results = -1;
  std::string scene2_key = obj_msg->get_scene(0)->get_key();

  // Iterate through pairs of scenes registered to
  for (int i = 0; i < num_scenes; i++) {
    // Get the scene ID's
    std::string scene1_key = registered_scenes->get_scene(i)->get_key();

    if (scene1_key != scene2_key) {
      // Get the user device transforms for each scene,
      // calculate the correct transform
      BaseQueryHelper::logger().debug("Calculating Transform");
      TransformInterface *new_trans = BaseQueryHelper::create_transform();
      // Start with the inverse of the destination transform
      new_trans->add_transform(\
          registered_scenes->get_scene(i)->get_device(0)->get_transform());
      new_trans->invert();
      // left multiply the origin transform
      new_trans->add_transform(\
          obj_msg->get_scene(0)->get_device(0)->get_transform());

      // Get any existing scene links
      BaseQueryHelper::logger().debug("Retrieving existing scene links");
      results = SceneQueryHelper::get_scene_link(scene1_key, scene2_key);
      if (results == 0) {
        // No links found, create new one
        SceneQueryHelper::create_scene_link(scene1_key, scene2_key, new_trans);
      } else if (results == 1) {
        // forward link found, update with correction
        SceneQueryHelper::update_scene_link(scene1_key, scene2_key, new_trans);
      } else if (results == 2) {
        // backward link found, update with correction
        new_trans->invert();
        SceneQueryHelper::update_scene_link(scene2_key, scene1_key, new_trans);
      }
    }
  }
}

// Try to find a path from one scene to the next and calculate
// the resulting transform
SceneTransformResult* AlgorithmQueryHelper::calculate_scene_scene_transform(\
  std::string scene_id1, std::string scene_id2) {
  BaseQueryHelper::logger().debug(\
    "Checking for existing paths between scenes: %s -> %s", \
    scene_id1, scene_id2);

  // Build the return transform result
  TransformInterface *new_tran = BaseQueryHelper::create_transform();
  SceneTransformResult *str = new SceneTransformResult(new_tran);
  // Allocate memory to hold the Neo4j Results
  Neocpp::ResultsIteratorInterface *results = NULL;
  Neocpp::ResultTreeInterface *tree = NULL;
  Neocpp::DbObjectInterface* obj = NULL;
  Neocpp::DbObjectInterface* path_obj = NULL;
  Neocpp::DbMapInterface *map = NULL;
  Neocpp::Neo4jQueryParameterInterface* pkey1_param = NULL;
  Neocpp::Neo4jQueryParameterInterface* pkey2_param = NULL;

  // Find the shortest path

  // The query string to find the path
  std::string path_q_string = "MATCH (base:Scene {key: {inp_key_start}}), "
    "(next:Scene {key: {inp_key_end}}), p = shortestPath((base)-[r*]-(next)) "
    "WHERE ALL (x IN nodes(p) WHERE (x:Scene)) RETURN p";

  // Set up the query parameters for query
  std::unordered_map<std::string, Neocpp::Neo4jQueryParameterInterface*> path_q_params;
  pkey1_param = \
    BaseQueryHelper::get_neo4j_factory()->get_neo4j_query_parameter(scene_id1);
  path_q_params.emplace("inp_key_start", pkey1_param);

  pkey2_param = \
    BaseQueryHelper::get_neo4j_factory()->get_neo4j_query_parameter(scene_id2);
  path_q_params.emplace("inp_key_start", pkey2_param);

  std::string err_str;
  bool has_exception = false;

  // Execute the query
  try {
    results = BaseQueryHelper::get_neo4j_interface()->execute(path_q_string, \
      path_q_params);
  }
  catch (std::exception& e) {
    BaseQueryHelper::logger().error("{\"Query\": \"%s\", \"Error\": \"%s\"", \
      path_q_string, e.what());
    err_str.assign(e.what());
    has_exception = true;
  }

  if (!results) {
    BaseQueryHelper::logger().error("No Path between scenes found");
    err_str = "No Path between scenes found";
    has_exception = true;
  } else if (!has_exception) {
    // If we have a path, then calculate the correct transform for the device

    // Get the path object from the query return
    tree = results->next();
    if (!tree) {
      BaseQueryHelper::logger().error("No results tree returned");
    } else {
      obj = tree->get(0);

      // Determine if we have a path in the return value
      if (obj->is_path()) {
        BaseQueryHelper::logger().debug("Path object detected in values from DB");

        // Iterate through the path
        int path_size = obj->size();
        TransformInterface *path_tran = NULL;
        for (int i = 0; i < path_size; i++) {
          // Allocate a new transform
          path_tran = BaseQueryHelper::create_transform();
          // Retrieve the path element
          path_obj = obj->get_path_element(i);

          // Are we dealing with an edge?
          if (path_obj->is_edge()) {

            // Get the property values
            map = path_obj->properties();
            if (map->element_exists("translation_x")) {
              path_tran->translate(0, map->get_float_element("translation_x"));
            }
            if (map->element_exists("translation_y")) {
              path_tran->translate(1, map->get_float_element("translation_y"));
            }
            if (map->element_exists("translation_z")) {
              path_tran->translate(2, map->get_float_element("translation_z"));
            }
            if (map->element_exists("rotation_x")) {
              path_tran->rotate(0, map->get_float_element("rotation_x"));
            }
            if (map->element_exists("rotation_y")) {
              path_tran->rotate(1, map->get_float_element("rotation_y"));
            }
            if (map->element_exists("rotation_z")) {
              path_tran->rotate(2, map->get_float_element("rotation_z"));
            }

            // Is our edge backward?
            if (!(path_obj->forward())) {
              path_tran->invert();
            }

            // Update the new transformation with the edge
            str->get_transform()->add_transform(path_tran);
          }
          if (path_obj) {delete path_obj; path_obj = NULL;}
          if (map) {delete map; map = NULL;}
          if (path_tran) {delete path_tran; path_tran = NULL;}
        }
      } else {
        BaseQueryHelper::logger().error("Non-Object value returned from query");
      }
    }
  }
  if (obj) delete obj;
  if (tree) delete tree;
  if (results) delete results;
  if (has_exception) throw QueryException(err_str);
  return str;
}
