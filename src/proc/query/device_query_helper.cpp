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

#include "include/device_query_helper.h"

// Determine if the given user device is registered to the given scene
bool DeviceQueryHelper::is_ud_registered(std::string inp_string, \
  std::string inp_device) {
  processor_logging->debug("Checking the User Devices registered to the Scene");
  ResultsIteratorInterface *results = NULL;
  ResultTreeInterface *tree = NULL;
  DbObjectInterface *obj = NULL;
  DbMapInterface *map = NULL;
  Neo4jQueryParameterInterface *key_param = NULL;
  // Create the query string
  std::string q_string =
    "MATCH (scn:Scene {key: {inp_key}})-[tr:TRANSFORM]->(ud:UserDevice)"
    " RETURN ud";

  // Set up the query parameters for query
  std::unordered_map<std::string, Neo4jQueryParameterInterface*> q_params;
  std::string qkey = inp_string;
  key_param = BaseQueryHelper::get_neo4j_factory()->get_neo4j_query_parameter(qkey);
  processor_logging->debug("Key:");
  processor_logging->debug(qkey);
  q_params.emplace("inp_key", key_param);

  // Execute the query
  bool ret_val = false;
  bool has_exception = false;
  std::string exc_string = "";
  try {
    results = BaseQueryHelper::get_neo4j_interface()->execute(q_string, q_params);
  }
  catch (std::exception& e) {
    processor_logging->error("Error running Query:");
    processor_logging->error(q_string);
    processor_logging->error(e.what());
    has_exception = true;
    std::string e_string(e.what());
    exc_string = e_string;
  }

  if (!results) {
    processor_logging->debug("User Device not found registered to scene");
    return false;
    // If we are registering the first device, then we may have a created scene
  } else {
    // Check if the registering user device is already registered
    processor_logging->debug("User Devices detected");
    tree = results->next();
    while (tree) {
      // Get the first DB Object (Node)
      obj = tree->get(0);
      processor_logging->debug("Query Result:");
      processor_logging->debug(obj->to_string());

      if (!(obj->is_node())) break;

      // Pull the node properties and assign them to the new
      // Scene object
      map = obj->properties();
      std::string db_key = "";
      if (map->element_exists("key")) {
        db_key = map->get_string_element("key");
      }

      if (db_key == inp_device) {
        processor_logging->debug("Existing registration detected");
        ret_val = true;
      }

      // Cleanup
      if (map) {delete map; map = NULL;}
      if (obj) {delete obj; obj = NULL;}
      if (tree) {delete tree; tree = NULL;}

      // Iterate to the next result
      tree = results->next();
    }
  }
  if (results) delete results;
  if (key_param) delete key_param;
  if (has_exception) throw QueryException(exc_string);
  return ret_val;
}

// Get scenes that this user device is registered to
// Collect the User Device and Transformation within the scenes returned
SceneListInterface* DeviceQueryHelper::get_registrations(std::string inp_device) {
  processor_logging->debug("Checking other scenes the device is registered to");
  ResultsIteratorInterface *results = NULL;
  ResultTreeInterface *tree = NULL;
  DbObjectInterface *obj = NULL;
  DbObjectInterface *edge = NULL;
  DbObjectInterface *device = NULL;
  DbMapInterface *edge_props = NULL;
  DbMapInterface* map = NULL;
  DbMapInterface *dev_props = NULL;
  Neo4jQueryParameterInterface *udkey_param = NULL;
  // Create the return object
  SceneListInterface *sc = NULL;
  if (BaseQueryHelper::get_config_manager()->get_formattype() == PROTO_FORMAT) {
    sc = BaseQueryHelper::create_protobuf_scene();
  } else {
    sc = BaseQueryHelper::create_json_scene();
  }
  // Create the query string
  std::string udq_string =
    "MATCH (scn:Scene)-[tr:TRANSFORM]->(ud:UserDevice {key: {inp_key}})"
    " RETURN scn, tr, ud";

  // Set up the query parameters for query
  std::unordered_map<std::string, Neo4jQueryParameterInterface*> udq_params;
  std::string udqkey = inp_device;
  udkey_param = BaseQueryHelper::get_neo4j_factory()->get_neo4j_query_parameter(udqkey);
  processor_logging->debug("Key:");
  processor_logging->debug(udqkey);
  udq_params.emplace("inp_key", udkey_param);

  // Execute the query
  try {
    results = BaseQueryHelper::get_neo4j_interface()->execute(udq_string, udq_params);
  }
  catch (std::exception& e) {
    processor_logging->error("Error running Query:");
    processor_logging->error(udq_string);
    processor_logging->error(e.what());
    return NULL;
  }

  int num_records = 0;
  if (!results) {
    processor_logging->debug("No Scenes found for the given device");
    return NULL;
  } else {
    // Iterate through the results
    // Build the scene list
    tree = results->next();
    while (tree) {
      processor_logging->debug("Record returned from results iterator");

      // Get the first DB Object (Node)
      obj = tree->get(0);
      if (!(obj->is_node()) && !(obj->is_edge())) break;
      edge = tree->get(1);
      if (!(edge->is_node()) && !(edge->is_edge())) break;
      device = tree->get(2);
      if (!(device->is_node()) && !(device->is_edge())) break;

      SceneInterface *new_data = BaseQueryHelper::create_scene();
      num_records = num_records + 1;
      processor_logging->debug("Query Result:");
      processor_logging->debug(obj->to_string());
      processor_logging->debug(edge->to_string());
      processor_logging->debug(device->to_string());

      // Pull the node properties and assign them to the new
      // Scene object
      processor_logging->debug("Getting Scene Properties");
      map = obj->properties();
      if (map->element_exists("key")) {
        new_data->set_key(map->get_string_element("key"));
      }
      if (map->element_exists("name")) {
        new_data->set_name(map->get_string_element("name"));
      }
      if (map->element_exists("latitude")) {
        new_data->set_latitude(map->get_float_element("latitude"));
      }
      if (map->element_exists("longitude")) {
        new_data->set_longitude(map->get_float_element("longitude"));
      }

      // Get the transform and device info
      double translation_x = -999.0;
      double translation_y = -999.0;
      double translation_z = -999.0;
      double rotation_x = -999.0;
      double rotation_y = -999.0;
      double rotation_z = -999.0;
      if (edge->is_edge())  {
        processor_logging->debug("Getting Edge Properties");
        edge_props = edge->properties();
        // Get the transform attributes
        if (edge_props->element_exists("translation_x")) {
          translation_x = edge_props->get_float_element("translation_x");
        }
        if (edge_props->element_exists("translation_y")) {
          translation_y = edge_props->get_float_element("translation_y");
        }
        if (edge_props->element_exists("translation_z")) {
          translation_z = edge_props->get_float_element("translation_z");
        }
        if (edge_props->element_exists("rotation_x")) {
          rotation_x = edge_props->get_float_element("rotation_x");
        }
        if (edge_props->element_exists("rotation_y")) {
          rotation_y = edge_props->get_float_element("rotation_y");
        }
        if (edge_props->element_exists("rotation_z")) {
          rotation_z = edge_props->get_float_element("rotation_z");
        }
      }

      if (device->is_node()) {
        processor_logging->debug("Getting Device Properties");
        // Get the transform and device properties
        dev_props = device->properties();

        if (dev_props) {
          if (dev_props->element_exists("key")) {
            processor_logging->debug("Device Key Found");
            // Add the device related data to the scene
            TransformInterface *new_transform = BaseQueryHelper::create_transform();
            UserDeviceInterface *new_dev = BaseQueryHelper::create_device(\
              dev_props->get_string_element("key"), new_transform);
            new_dev->set_translation(0, translation_x);
            new_dev->set_translation(1, translation_y);
            new_dev->set_translation(2, translation_z);
            new_dev->set_rotation(0, rotation_x);
            new_dev->set_rotation(1, rotation_y);
            new_dev->set_rotation(2, rotation_z);
            processor_logging->debug("Adding device to scene data");
            new_data->add_device(new_dev);
          }
        } else {
          processor_logging->error("Null map returned for device properties");
        }
      }

      processor_logging->debug("Adding Scene data to scene");
      sc->add_scene(new_data);

      processor_logging->debug("Cleanup");
      if (edge_props) {delete edge_props; edge_props = NULL;}
      if (dev_props) {delete dev_props; dev_props = NULL;}
      if (map) {delete map; map = NULL;}
      if (obj) {delete obj; obj = NULL;}
      if (edge) {delete edge; edge = NULL;}
      if (device) {delete device; device = NULL;}
      if (tree) {delete tree; tree = NULL;}

      processor_logging->debug("Getting next result");
      tree = results->next();
    }
  }
  if (results) delete results;
  if (udkey_param) delete udkey_param;
  if (num_records == 0) {
    processor_logging->error("No Scenes found for the given device");
    return NULL;
  }
  return sc;
}

// Create a registration link in the DB
void DeviceQueryHelper::register_device_to_scene(std::string device_id, \
  std::string scene_id, TransformInterface *transform) {
  processor_logging->debug("Creating Device Registration link");
  ResultsIteratorInterface *results = NULL;
  Neo4jQueryParameterInterface *skey_param = NULL;
  Neo4jQueryParameterInterface *udkey_param = NULL;
  Neo4jQueryParameterInterface *locx_param = NULL;
  Neo4jQueryParameterInterface *locy_param = NULL;
  Neo4jQueryParameterInterface *locz_param = NULL;
  Neo4jQueryParameterInterface *rotx_param = NULL;
  Neo4jQueryParameterInterface *roty_param = NULL;
  Neo4jQueryParameterInterface *rotz_param = NULL;

  // Create the query string
  std::string udq_string =
    "MATCH (scn:Scene {key: {inp_key}}) "
    "CREATE (scn)-[trans:TRANSFORM {translation_x: {loc_x}, "
      "translation_y: {loc_y}, translation_z: {loc_z}, rotation_x: {rot_x}, "
      "rotation_y: {rot_y}, rotation_z: {rot_z}}]->(ud:UserDevice "
      "{key: {inp_ud_key}}) "
    "RETURN scn, trans, ud";

  // Set up the query parameters for query
  std::unordered_map<std::string, Neo4jQueryParameterInterface*> q_params;

  // Insert the scene key into the query list
  skey_param = BaseQueryHelper::get_neo4j_factory()->get_neo4j_query_parameter(scene_id);
  processor_logging->debug("Scene Key:");
  processor_logging->debug(scene_id);
  q_params.emplace("inp_key", skey_param);

  // Insert the device key into the query list
  udkey_param = BaseQueryHelper::get_neo4j_factory()->get_neo4j_query_parameter(device_id);
  processor_logging->debug("Device Key:");
  processor_logging->debug(device_id);
  q_params.emplace("inp_ud_key", udkey_param);

  // Insert translation
  locx_param = BaseQueryHelper::get_neo4j_factory()->get_neo4j_query_parameter(transform->translation(0));
  locy_param = BaseQueryHelper::get_neo4j_factory()->get_neo4j_query_parameter(transform->translation(1));
  locz_param = BaseQueryHelper::get_neo4j_factory()->get_neo4j_query_parameter(transform->translation(2));
  q_params.emplace("loc_x", locx_param);
  q_params.emplace("loc_y", locy_param);
  q_params.emplace("loc_z", locz_param);

  // Insert rotation
  rotx_param = BaseQueryHelper::get_neo4j_factory()->get_neo4j_query_parameter(transform->rotation(0));
  roty_param = BaseQueryHelper::get_neo4j_factory()->get_neo4j_query_parameter(transform->rotation(1));
  rotz_param = BaseQueryHelper::get_neo4j_factory()->get_neo4j_query_parameter(transform->rotation(2));
  q_params.emplace("rot_x", rotx_param);
  q_params.emplace("rot_y", roty_param);
  q_params.emplace("rot_z", rotz_param);

  // Execute the query
  try {
    results = BaseQueryHelper::get_neo4j_interface()->execute(udq_string, q_params);
  }
  catch (std::exception& e) {
    processor_logging->error("Error running Query:");
    processor_logging->error(udq_string);
    processor_logging->error(e.what());
  }

  if (!results) {
    processor_logging->error("No Links created");
  } else {
    delete results;
  }
  if (skey_param) delete skey_param;
  if (udkey_param) delete udkey_param;
  if (locx_param) delete locx_param;
  if (locy_param) delete locy_param;
  if (locz_param) delete locz_param;
  if (rotx_param) delete rotx_param;
  if (roty_param) delete roty_param;
  if (rotz_param) delete rotz_param;
}

// Remove a registration link in the DB between the specified device and scene
void DeviceQueryHelper::remove_device_from_scene(std::string device_id, \
  std::string scene_id) {
  // Set up the Cypher Query for device deletion
  ResultsIteratorInterface *results = NULL;
  Neo4jQueryParameterInterface* skey_param = NULL;
  Neo4jQueryParameterInterface* udkey_param = NULL;
  std::string query_string =
    "MATCH (scn:Scene {key: {inp_key}})-[trans:TRANSFORM]->"
      "(ud:UserDevice {key: {inp_ud_key}}) "
    "DETACH DELETE ud RETURN scn";
  processor_logging->debug("Executing Delete Query");

  // Set up the query parameters for query
  std::unordered_map<std::string, Neo4jQueryParameterInterface*> q_params;

  // Insert the scene key into the query list
  skey_param = BaseQueryHelper::get_neo4j_factory()->get_neo4j_query_parameter(scene_id);
  processor_logging->debug("Scene Key:");
  processor_logging->debug(scene_id);
  q_params.emplace("inp_key", skey_param);

  // Insert the device key into the query list
  udkey_param = BaseQueryHelper::get_neo4j_factory()->get_neo4j_query_parameter(device_id);
  processor_logging->debug("Device Key:");
  processor_logging->debug(device_id);
  q_params.emplace("inp_ud_key", udkey_param);

  // Execute the query
  try {
    results = BaseQueryHelper::get_neo4j_interface()->execute(query_string, q_params);
  }
  catch (std::exception& e) {
    processor_logging->error("Error running Query:");
    processor_logging->error(query_string);
    processor_logging->error(e.what());
  }

  if (!results) processor_logging->error("No Links destroyed");
  else
    {delete results;}
  if (skey_param) delete skey_param;
  if (udkey_param) delete udkey_param;
}

// Update the device registration
// Return true if the update was successful
// False if we were unable to find an existing link to update
bool DeviceQueryHelper::update_device_registration(std::string dev_id, \
  std::string scene_id, TransformInterface *transform) {
  processor_logging->debug("Updating Device Registration link");
  ResultsIteratorInterface *results = NULL;
  ResultTreeInterface *tree = NULL;
  DbObjectInterface *obj = NULL;
  Neo4jQueryParameterInterface *skey_param = NULL;
  Neo4jQueryParameterInterface *udkey_param = NULL;
  Neo4jQueryParameterInterface *locx_param = NULL;
  Neo4jQueryParameterInterface *locy_param = NULL;
  Neo4jQueryParameterInterface *locz_param = NULL;
  Neo4jQueryParameterInterface *rotx_param = NULL;
  Neo4jQueryParameterInterface *roty_param = NULL;
  Neo4jQueryParameterInterface *rotz_param = NULL;
  // Create the query string
  std::string udq_string =
    "MATCH (scn:Scene {key: {inp_key}})-[trans:TRANSFORM]->"
      "(ud:UserDevice  {key: {inp_ud_key}}) "
    "SET trans.translation_x = {loc_x}, trans.translation_y = {loc_y}, "
      "trans.translation_z = {loc_z}, trans.rotation_x = {rot_x}, "
      "trans.rotation_y = {rot_y}, trans.rotation_z = {rot_z} "
    "RETURN scn, trans, ud";

  // Set up the query parameters for query
  std::unordered_map<std::string, Neo4jQueryParameterInterface*> q_params;

  // Insert the scene key into the query list
  skey_param = BaseQueryHelper::get_neo4j_factory()->get_neo4j_query_parameter(scene_id);
  processor_logging->debug("Scene Key:");
  processor_logging->debug(scene_id);
  q_params.emplace("inp_key", skey_param);

  // Insert the device key into the query list
  udkey_param = BaseQueryHelper::get_neo4j_factory()->get_neo4j_query_parameter(dev_id);
  processor_logging->debug("Device Key:");
  processor_logging->debug(dev_id);
  q_params.emplace("inp_ud_key", udkey_param);

  // Insert translation
  locx_param = BaseQueryHelper::get_neo4j_factory()->get_neo4j_query_parameter(transform->translation(0));
  locy_param = BaseQueryHelper::get_neo4j_factory()->get_neo4j_query_parameter(transform->translation(1));
  locz_param = BaseQueryHelper::get_neo4j_factory()->get_neo4j_query_parameter(transform->translation(2));
  q_params.emplace("loc_x", locx_param);
  q_params.emplace("loc_y", locy_param);
  q_params.emplace("loc_z", locz_param);

  // Insert rotation
  rotx_param = BaseQueryHelper::get_neo4j_factory()->get_neo4j_query_parameter(transform->rotation(0));
  roty_param = BaseQueryHelper::get_neo4j_factory()->get_neo4j_query_parameter(transform->rotation(1));
  rotz_param = BaseQueryHelper::get_neo4j_factory()->get_neo4j_query_parameter(transform->rotation(2));
  q_params.emplace("rot_x", rotx_param);
  q_params.emplace("rot_y", roty_param);
  q_params.emplace("rot_z", rotz_param);

  processor_logging->debug("Executing Query:");

  // Execute the query
  bool has_exception = false;
  bool result_found = true;
  std::string exc_string = "";
  try {
    results = BaseQueryHelper::get_neo4j_interface()->execute(udq_string, q_params);
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
    result_found = false;
  } else if ((!has_exception) && result_found) {
    processor_logging->debug("Query Executed Successfully");
    tree = results->next();
    if (!tree) {
      processor_logging->error("Query Returned no result tree");
      result_found = false;
    } else {
      obj = tree->get(0);
      if (!(obj->is_node())) {
        processor_logging->error("Query Returned no values");
        result_found = false;
      } else {processor_logging->debug(obj->to_string());}
    }
  }
  if (obj) delete obj;
  if (tree) delete tree;
  if (results) delete results;
  if (skey_param) delete skey_param;
  if (udkey_param) delete udkey_param;
  if (locx_param) delete locx_param;
  if (locy_param) delete locy_param;
  if (locz_param) delete locz_param;
  if (rotx_param) delete rotx_param;
  if (roty_param) delete roty_param;
  if (rotz_param) delete rotz_param;
  if (has_exception) throw QueryException(exc_string);
  return result_found;
}
