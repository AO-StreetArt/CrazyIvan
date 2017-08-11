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

#include "include/crud_message_processor.h"

// -------------------------------------------------------------------------- //
// ----------------------------CRUD Support---------------------------------- //
// -------------------------------------------------------------------------- //

// Create a new scene
ProcessResult* CrudMessageProcessor::process_create_message(SceneListInterface *obj_msg) {
  processor_logging->info("Processing Scene Creation message");
  processor_logging->debug("Number of Scene Data elements:");
  processor_logging->debug(obj_msg->num_scenes());
  ProcessResult *response = new ProcessResult;
  if (obj_msg->num_scenes() > 0) {
    ResultsIteratorInterface *results = NULL;
    Neo4jQueryParameterInterface *name_param = NULL;
    Neo4jQueryParameterInterface *lat_param = NULL;
    Neo4jQueryParameterInterface *long_param = NULL;
    Neo4jQueryParameterInterface *key_param = NULL;
    ResultTreeInterface *tree = NULL;
    DbObjectInterface* obj = NULL;
    double qlat;
    double qlong;
    processor_logging->info("Scenes Found in creation message");

    // Empty string to hold a newly generated ID, if needed
    std::string new_id;

    // Set up the Cypher Query for scene creation
    std::string scene_query = "CREATE (scn:Scene {key: {inp_key}";

    // Set up the query parameters for scene creation
    std::unordered_map<std::string, Neo4jQueryParameterInterface*> scene_params;
    if (!(obj_msg->get_scene(0)->get_key().empty())) {
      // Use an existing key from the input message
      key_param = \
        BaseMessageProcessor::get_neo4j_factory()->get_neo4j_query_parameter(\
        obj_msg->get_scene(0)->get_key());
    } else {
      // Generate a new key
      BaseMessageProcessor::create_uuid(new_id);
      if (new_id.empty()) {
        processor_logging->error("Unknown error generating new key for scene");
        response->set_error(PROCESSING_ERROR, "Error generating key for scene");
        return response;
      }
      processor_logging->info("Key Generated:");
      processor_logging->info(new_id);
      key_param = BaseMessageProcessor::get_neo4j_factory()->get_neo4j_query_parameter(new_id);
    }
    scene_params.emplace("inp_key", key_param);
    std::string qname = obj_msg->get_scene(0)->get_name();
    if (!(qname.empty())) {
      name_param = BaseMessageProcessor::get_neo4j_factory()->get_neo4j_query_parameter(qname);
      processor_logging->debug("Name:");
      processor_logging->debug(qname);
      scene_params.emplace("inp_name", name_param);
      scene_query = scene_query + ", name: {inp_name}";
    }
    if (!(obj_msg->get_scene(0)->get_latitude() == -9999.0)) {
      qlat = obj_msg->get_scene(0)->get_latitude();
      lat_param = BaseMessageProcessor::get_neo4j_factory()->get_neo4j_query_parameter(qlat);
      processor_logging->debug("Latitude:");
      processor_logging->debug(std::to_string(qlat));
      scene_params.emplace("inp_lat", lat_param);
      scene_query = scene_query + ", latitude: {inp_lat}";
    }
    if (!(obj_msg->get_scene(0)->get_longitude() == -9999.0)) {
      qlong = obj_msg->get_scene(0)->get_longitude();
      long_param = BaseMessageProcessor::get_neo4j_factory()->get_neo4j_query_parameter(qlong);
      processor_logging->debug("Longitude:");
      processor_logging->debug(std::to_string(qlong));
      scene_params.emplace("inp_long", long_param);
      scene_query = scene_query + ", longitude: {inp_long}";
    }

    scene_query = scene_query + "}) RETURN scn";
    processor_logging->debug(scene_query);

    // Execute the query
    try {
      results = BaseMessageProcessor::get_neo4j_interface()->execute(scene_query, scene_params);
    }
    catch (std::exception& e) {
      processor_logging->error("Error running Query:");
      processor_logging->error(e.what());
      response->set_error(PROCESSING_ERROR, e.what());
    }

    if (!results) {
      processor_logging->error("No results returned from create query");
      response->set_error(PROCESSING_ERROR, "Error processing Scene Update");
    } else {
      processor_logging->info("Scene Successfully added");
      response->set_return_string(new_id);
      tree = results->next();
      if (tree) {
        obj = tree->get(0);
        if (obj) {
          if (!(obj->is_node())) {
            response->set_error(NOT_FOUND, "Document not found");
          }
          delete obj;
        }
        delete tree;
      }
    }

    if (results) delete results;
    if (name_param) delete name_param;
    if (lat_param) delete lat_param;
    if (long_param) delete long_param;
    if (key_param) delete key_param;
    return response;
  }
  response->set_error(PROCESSING_ERROR, "No Scene Data recieved");
  return response;
}

// Update the details of a scene entry
ProcessResult* CrudMessageProcessor::process_update_message(SceneListInterface *obj_msg) {
  ProcessResult *response = new ProcessResult;

  if (obj_msg->num_scenes() > 0) {
    // Retrieve a Redis Mutex Lock on the scene
    if (BaseMessageProcessor::get_config_manager()->get_atomictransactions()) {
      get_mutex_lock(obj_msg->get_scene(0)->get_key());
    }

    // Declare base variables
    bool is_started = false;
    ResultsIteratorInterface *results = NULL;
    Neo4jQueryParameterInterface *name_param = NULL;
    Neo4jQueryParameterInterface *lat_param = NULL;
    Neo4jQueryParameterInterface *long_param = NULL;
    Neo4jQueryParameterInterface *key_param = NULL;
    processor_logging->debug("Processing Scene Update message");

    // Ensure that we have fields in the query
    if ((obj_msg->get_scene(0)->get_name().empty() && \
      obj_msg->get_scene(0)->get_latitude() == -9999.0 && \
      obj_msg->get_scene(0)->get_longitude() == -9999.0) || \
      obj_msg->get_scene(0)->get_key().empty()) {
      processor_logging->error("No fields found in update message");
      response->set_error(INSUFF_DATA_ERROR, "Insufficient fields in message");
    } else {
      // Set up the Cypher Query for scene update
      std::string scene_query = "MATCH (scn:Scene {key: {inp_key}}) SET ";
      if (!(obj_msg->get_scene(0)->get_name().empty())) {
        scene_query = scene_query + "scn.name = {inp_name}";
        is_started = true;
      }

      if (!(obj_msg->get_scene(0)->get_latitude() == -9999.0)) {
        if (is_started) {
          scene_query = scene_query + ", ";
        }
        scene_query = scene_query + "scn.latitude = {inp_lat}";
        is_started = true;
      }

      if (!(obj_msg->get_scene(0)->get_longitude() == -9999.0)) {
        if (is_started) {
          scene_query = scene_query + ", ";
        }
        scene_query = scene_query + "scn.longitude = {inp_long}";
        is_started = true;
      }

      scene_query = scene_query + " RETURN scn";

      processor_logging->debug("Executing Query:");
      processor_logging->debug(scene_query);

      // Set up the query parameters for scene update
      std::unordered_map<std::string, Neo4jQueryParameterInterface*> \
        scene_params;
      std::string qkey = obj_msg->get_scene(0)->get_key();
      key_param = BaseMessageProcessor::get_neo4j_factory()->get_neo4j_query_parameter(qkey);
      processor_logging->debug("Key:");
      processor_logging->debug(qkey);
      scene_params.emplace("inp_key", key_param);
      if (!(obj_msg->get_scene(0)->get_name().empty())) {
        std::string qname = obj_msg->get_scene(0)->get_name();
        name_param = BaseMessageProcessor::get_neo4j_factory()->get_neo4j_query_parameter(qname);
        processor_logging->debug("Name:");
        processor_logging->debug(qname);
        scene_params.emplace("inp_name", name_param);
      }
      if (!(obj_msg->get_scene(0)->get_latitude() == -9999.0)) {
        double qlat = obj_msg->get_scene(0)->get_latitude();
        lat_param = BaseMessageProcessor::get_neo4j_factory()->get_neo4j_query_parameter(qlat);
        scene_params.emplace("inp_lat", lat_param);
      }
      if (!(obj_msg->get_scene(0)->get_longitude() == -9999.0)) {
        double qlong = obj_msg->get_scene(0)->get_longitude();
        long_param = BaseMessageProcessor::get_neo4j_factory()->get_neo4j_query_parameter(qlong);
        scene_params.emplace("inp_long", long_param);
      }

      // Execute the query
      ResultTreeInterface *tree = NULL;
      DbObjectInterface* obj = NULL;
      try {
        results = BaseMessageProcessor::get_neo4j_interface()->execute(scene_query, scene_params);
      }
      catch (std::exception& e) {
        if (obj) delete obj;
        if (tree) delete tree;
        processor_logging->error("Error running Query:");
        processor_logging->error(e.what());
        response->set_error(PROCESSING_ERROR, e.what());
      }

      if (!results) {
        processor_logging->error("No results returned from update query");
        response->set_error(PROCESSING_ERROR, "Error processing Scene Get");
      } else {
        response->set_return_string(qkey);
        tree = results->next();
        if (tree) {
          obj = tree->get(0);
          if (obj) {
            if (!(obj->is_node())) {
              response->set_error(NOT_FOUND, "Document not found");
            }
            delete obj;
          }
          delete tree;
        }
      }
    }

    // Cleanup
    if (results) delete results;
    if (name_param) delete name_param;
    if (lat_param) delete lat_param;
    if (long_param) delete long_param;
    if (key_param) delete key_param;

    // Release the Redis Mutex Lock
    if (BaseMessageProcessor::get_config_manager()->get_atomictransactions()) {
      release_mutex_lock(obj_msg->get_scene(0)->get_key());
    }
    return response;
  }

  response->set_error(PROCESSING_ERROR, "No Scene Data recieved");
  return response;
}

// Query for scene data
ProcessResult* CrudMessageProcessor::process_retrieve_message(SceneListInterface *obj_msg) {
  ProcessResult *response = new ProcessResult;
  if (obj_msg->num_scenes() > 0) {
    ResultsIteratorInterface *results = NULL;
    Neo4jQueryParameterInterface *name_param = NULL;
    Neo4jQueryParameterInterface *lat_param = NULL;
    Neo4jQueryParameterInterface *long_param = NULL;
    Neo4jQueryParameterInterface *key_param = NULL;
    Neo4jQueryParameterInterface *dist_param = NULL;
    processor_logging->debug("Processing Scene Retrieve message");

    if (obj_msg->get_scene(0)->get_name().empty() && \
      obj_msg->get_scene(0)->get_latitude() == -9999.0 && \
      obj_msg->get_scene(0)->get_longitude() == -9999.0 && \
      obj_msg->get_scene(0)->get_key().empty()) {
      processor_logging->error("No fields found in get message");
      response->set_error(INSUFF_DATA_ERROR, "Insufficient fields for get");
    } else {
      // Set up the Cypher Query for scene retrieval
      std::string scene_query = "MATCH (scn:Scene";

      if (!(obj_msg->get_scene(0)->get_key().empty())) {
        scene_query = scene_query + " {key: {inp_key}})";
      } else {
        if (!(obj_msg->get_scene(0)->get_name().empty())) {
          scene_query = scene_query + " {name: {inp_name}}";
        }

        scene_query = scene_query + ")";

        if (!(obj_msg->get_scene(0)->get_latitude() == -9999.0 || \
          obj_msg->get_scene(0)->get_longitude() == -9999.0 || \
          obj_msg->get_scene(0)->get_distance() < 0.0)) {
          // Query for distance
          // Assumes distance supplied is in meters
          // Haversine formula (https://en.wikipedia.org/wiki/Haversine_formula)
          std::string where_clause = \
                " WHERE ( "
                  "12742000 * asin("
                    "sqrt("
                      "haversin(radians({inp_lat} - scn.latitude)) + "
                      "cos(radians({inp_lat})) * cos(radians(scn.latitude)) * "
                        "haversin(radians(scn.longitude - {inp_long}))"
                    ")"
                  ")"
                ") < {inp_distance}";
          scene_query = scene_query + where_clause;
        }
      }

      scene_query = scene_query + " RETURN scn";

      processor_logging->debug("Executing Query:");
      processor_logging->debug(scene_query);

      // Set up the query parameters for scene retrieval
      std::unordered_map<std::string, Neo4jQueryParameterInterface*> \
        scene_params;
      if (!(obj_msg->get_scene(0)->get_key().empty())) {
        std::string qkey = obj_msg->get_scene(0)->get_key();
        key_param = BaseMessageProcessor::get_neo4j_factory()->get_neo4j_query_parameter(qkey);
        processor_logging->debug("Key:");
        processor_logging->debug(qkey);
        scene_params.emplace("inp_key", key_param);
      }
      if (!(obj_msg->get_scene(0)->get_name().empty())) {
        std::string qname = obj_msg->get_scene(0)->get_name();
        name_param = BaseMessageProcessor::get_neo4j_factory()->get_neo4j_query_parameter(qname);
        processor_logging->debug("Name:");
        processor_logging->debug(qname);
        scene_params.emplace("inp_name", name_param);
      }
      if (!(obj_msg->get_scene(0)->get_latitude() == -9999.0 || \
        obj_msg->get_scene(0)->get_longitude() == -9999.0 || \
        obj_msg->get_scene(0)->get_distance() < 0.0)) {
        double qlat = obj_msg->get_scene(0)->get_latitude();
        lat_param = BaseMessageProcessor::get_neo4j_factory()->get_neo4j_query_parameter(qlat);
        scene_params.emplace("inp_lat", lat_param);
        double qlong = obj_msg->get_scene(0)->get_longitude();
        long_param = BaseMessageProcessor::get_neo4j_factory()->get_neo4j_query_parameter(qlong);
        scene_params.emplace("inp_long", long_param);
        double qdist = obj_msg->get_scene(0)->get_distance();
        dist_param = BaseMessageProcessor::get_neo4j_factory()->get_neo4j_query_parameter(qdist);
        scene_params.emplace("inp_distance", dist_param);
      }

      // Execute the query
      try {
        results = BaseMessageProcessor::get_neo4j_interface()->execute(scene_query, scene_params);
        if (!results) {
          processor_logging->error("No results returned from query");
          response->set_error(PROCESSING_ERROR, "Error processing Scene Get");
        } else {
          // Pull results and return
          SceneListInterface *sc = NULL;
          if (BaseMessageProcessor::get_config_manager()->get_formattype() == PROTO_FORMAT) \
            {sc = BaseMessageProcessor::get_slfactory().build_protobuf_scene();}
          else {sc = BaseMessageProcessor::get_slfactory().build_json_scene();}
          sc->set_err_code(NO_ERROR);
          sc->set_msg_type(SCENE_GET);
          sc->set_transaction_id(obj_msg->get_transaction_id());
          ResultTreeInterface *tree = results->next();
          int num_results = 0;
          while (tree) {
            SceneInterface *data = BaseMessageProcessor::get_sfactory().build_scene();

            // Get the first DB Object (Node)
            DbObjectInterface* obj = tree->get(0);
            processor_logging->debug("Query Result:");
            processor_logging->debug(obj->to_string());

            // Leave the loop if we don't have anything in this result tree
            if (!(obj->is_node()) && !(obj->is_edge())) break;
            num_results = num_results + 1;

            // Pull the node properties and assign them to the new
            // Scene object
            DbMapInterface* map = obj->properties();
            if (map->element_exists("key")) {
              std::string new_key = map->get_string_element("key", 512);
              processor_logging->debug("Key retrieved from query:");
              processor_logging->debug(new_key);
              data->set_key(new_key);
            }
            if (map->element_exists("name")) {
              data->set_name(map->get_string_element("name", 512));
            }
            if (map->element_exists("latitude")) {
              data->set_latitude(map->get_float_element("latitude"));
            }
            if (map->element_exists("longitude")) {
              data->set_longitude(map->get_float_element("longitude"));
            }

            sc->add_scene(data);

            // Iterate to the next result
            if (tree) delete tree;
            if (obj) delete obj;
            if (map) delete map;
            tree = results->next();
          }
          if (num_results > 0) {
            processor_logging->debug("Response Scene List:");
            sc->print();
            std::string msg_string;
            sc->to_msg_string(msg_string);
            response->set_return_string(msg_string);
          } else {
            response->set_error(NOT_FOUND, "Document not found");
          }
        }
      }
      catch (std::exception& e) {
        processor_logging->error("Error running Query:");
        processor_logging->error(e.what());
        response->set_error(PROCESSING_ERROR, e.what());
      }
    }
    if (results) delete results;
    if (name_param) delete name_param;
    if (lat_param) delete lat_param;
    if (long_param) delete long_param;
    if (key_param) delete key_param;
    if (dist_param) delete dist_param;
    return response;
  }
  response->set_error(PROCESSING_ERROR, "No Scene Data recieved");
  return response;
}

// Delete a scene
ProcessResult* CrudMessageProcessor::process_delete_message(SceneListInterface *obj_msg) {
  ProcessResult *response = new ProcessResult;
  if (obj_msg->num_scenes() > 0) {
    processor_logging->debug("Processing Scene Delete message");
    ResultsIteratorInterface *results = NULL;
    Neo4jQueryParameterInterface *key_param = NULL;

    if (obj_msg->get_scene(0)->get_key().empty()) {
      processor_logging->error("No fields found in delete message");
      response->set_error(INSUFF_DATA_ERROR, "Insufficient fields in message");
    } else {
      // Set up the Cypher Query for scene delete
      std::string scene_query = \
        "MATCH (scn:Scene {key: {inp_key}}) DETACH DELETE scn RETURN scn";
      processor_logging->debug("Executing Delete Query");

      // Set up the query parameters for scene delete
      std::unordered_map<std::string, Neo4jQueryParameterInterface*> \
        scene_params;
      std::string qkey = obj_msg->get_scene(0)->get_key();
      key_param = BaseMessageProcessor::get_neo4j_factory()->get_neo4j_query_parameter(qkey);
      processor_logging->debug("Key:");
      processor_logging->debug(qkey);
      scene_params.emplace("inp_key", key_param);

      // Execute the query
      ResultTreeInterface *tree = NULL;
      DbObjectInterface* obj = NULL;
      try {
        results = BaseMessageProcessor::get_neo4j_interface()->execute(scene_query, scene_params);
      }
      catch (std::exception& e) {
        processor_logging->error("Error running Query:");
        processor_logging->error(e.what());
        response->set_error(PROCESSING_ERROR, e.what());
      }
      if (!results) {
        processor_logging->error("No results returned from delete query");
        response->set_error(PROCESSING_ERROR, "Error processing Scene Get");
      } else {
        response->set_return_string(qkey);
        tree = results->next();
        if (tree) {
          obj = tree->get(0);
          if (obj) {
            if (!(obj->is_node())) {
              response->set_error(NOT_FOUND, "Document not found");
            }
            delete obj;
          }
          delete tree;
        }
      }
    }
    if (results) delete results;
    if (key_param) delete key_param;
    return response;
  }
  response->set_error(PROCESSING_ERROR, "No Scene Data recieved");
  return response;
}
