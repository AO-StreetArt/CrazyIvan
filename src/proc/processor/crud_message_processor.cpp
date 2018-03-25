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
ProcessResult* \
  CrudMessageProcessor::process_create_message(SceneListInterface *obj_msg) {
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
    Neo4jQueryParameterInterface *region_param = NULL;
    Neo4jQueryParameterInterface *asset_param = NULL;
    Neo4jQueryParameterInterface *tag_param = NULL;
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
    // Key
    if (!(obj_msg->get_scene(0)->get_key().empty())) {
      // Use an existing key from the input message
      key_param = \
        BaseMessageProcessor::get_neo4j_factory()->get_neo4j_query_parameter(\
        obj_msg->get_scene(0)->get_key());
      response->set_return_string(obj_msg->get_scene(0)->get_key());
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
      key_param = BaseMessageProcessor::get_neo4j_factory()->\
        get_neo4j_query_parameter(new_id);
      response->set_return_string(new_id);
    }
    scene_params.emplace("inp_key", key_param);
    std::string qname = obj_msg->get_scene(0)->get_name();
    if (!(qname.empty())) {
      name_param = BaseMessageProcessor::get_neo4j_factory()->\
        get_neo4j_query_parameter(qname);
      processor_logging->debug("Name:");
      processor_logging->debug(qname);
      scene_params.emplace("inp_name", name_param);
      scene_query = scene_query + ", name: {inp_name}";
    }
    // Latitude
    if (!(obj_msg->get_scene(0)->get_latitude() == -9999.0)) {
      qlat = obj_msg->get_scene(0)->get_latitude();
      lat_param = BaseMessageProcessor::get_neo4j_factory()->\
        get_neo4j_query_parameter(qlat);
      processor_logging->debug("Latitude:");
      processor_logging->debug(std::to_string(qlat));
      scene_params.emplace("inp_lat", lat_param);
      scene_query = scene_query + ", latitude: {inp_lat}";
    }
    // Longitude
    if (!(obj_msg->get_scene(0)->get_longitude() == -9999.0)) {
      qlong = obj_msg->get_scene(0)->get_longitude();
      long_param = BaseMessageProcessor::get_neo4j_factory()->\
        get_neo4j_query_parameter(qlong);
      processor_logging->debug("Longitude:");
      processor_logging->debug(std::to_string(qlong));
      scene_params.emplace("inp_long", long_param);
      scene_query = scene_query + ", longitude: {inp_long}";
    }
    // Region
    if (!(obj_msg->get_scene(0)->get_region().empty())) {
      region_param = BaseMessageProcessor::get_neo4j_factory()->\
        get_neo4j_query_parameter(obj_msg->get_scene(0)->get_region());
      processor_logging->debug("Region:");
      processor_logging->debug(obj_msg->get_scene(0)->get_region());
      scene_params.emplace("inp_region", region_param);
      scene_query = scene_query + ", region: {inp_region}";
    }
    // Assets
    if (obj_msg->get_scene(0)->num_assets() > 0) {
      // Create the Query Parameter
      asset_param = BaseMessageProcessor::get_neo4j_factory()->\
        get_neo4j_query_parameter();
      // Add the assets from the object message to the parameter
      for (int i = 0; i < obj_msg->get_scene(0)->num_assets(); i++) {
        asset_param->add_value(obj_msg->get_scene(0)->get_asset(i));
        processor_logging->debug("Asset:");
        processor_logging->debug(obj_msg->get_scene(0)->get_asset(i));
      }
      // Put the query parameter and query section in place
      scene_params.emplace("inp_assets", asset_param);
      scene_query = scene_query + ", assets: {inp_assets}";
    }
    // Tags
    if (obj_msg->get_scene(0)->num_tags() > 0) {
      // Create the Query Parameter
      tag_param = BaseMessageProcessor::get_neo4j_factory()->\
        get_neo4j_query_parameter();
      // Add the assets from the object message to the parameter
      for (int j = 0; j < obj_msg->get_scene(0)->num_tags(); j++) {
        tag_param->add_value(obj_msg->get_scene(0)->get_tag(j));
        processor_logging->debug("Tag:");
        processor_logging->debug(obj_msg->get_scene(0)->get_tag(j));
      }
      // Put the query parameter and query section in place
      scene_params.emplace("inp_tags", tag_param);
      scene_query = scene_query + ", tags: {inp_tags}";
    }

    scene_query = scene_query + "}) RETURN scn";
    processor_logging->debug(scene_query);

    // Execute the query
    try {
      results = BaseMessageProcessor::get_neo4j_interface()->\
        execute(scene_query, scene_params);
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
    if (asset_param) delete asset_param;
    if (tag_param) delete tag_param;
    if (region_param) delete region_param;
    return response;
  }
  response->set_error(PROCESSING_ERROR, "No Scene Data recieved");
  return response;
}

// Update the details of a scene entry
ProcessResult* \
  CrudMessageProcessor::process_update_message(SceneListInterface *obj_msg) {
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
    Neo4jQueryParameterInterface *region_param = NULL;
    Neo4jQueryParameterInterface *asset_param = NULL;
    Neo4jQueryParameterInterface *tag_param = NULL;
    processor_logging->debug("Processing Scene Update message");

    // Ensure that we have fields in the query
    if (obj_msg->get_scene(0)->get_name().empty() && \
      obj_msg->get_scene(0)->get_latitude() == -9999.0 && \
      obj_msg->get_scene(0)->get_longitude() == -9999.0 && \
      obj_msg->get_scene(0)->get_key().empty() && \
      obj_msg->get_scene(0)->get_region().empty() && \
      obj_msg->get_scene(0)->num_tags() == 0 && \
      obj_msg->get_scene(0)->num_assets() == 0) {
      processor_logging->error("No fields found in update message");
      response->set_error(INSUFF_DATA_ERROR, "Insufficient fields in message");
    } else {
      // Set up the Cypher Query for scene update
      std::string scene_query = "MATCH (scn:Scene {key: {inp_key}}) SET ";
      // Name
      if (!(obj_msg->get_scene(0)->get_name().empty())) {
        scene_query = scene_query + "scn.name = {inp_name}";
        is_started = true;
      }
      // Latitude
      if (!(obj_msg->get_scene(0)->get_latitude() == -9999.0)) {
        if (is_started) {
          scene_query = scene_query + ", ";
        }
        scene_query = scene_query + "scn.latitude = {inp_lat}";
        is_started = true;
      }
      // Longitude
      if (!(obj_msg->get_scene(0)->get_longitude() == -9999.0)) {
        if (is_started) {
          scene_query = scene_query + ", ";
        }
        scene_query = scene_query + "scn.longitude = {inp_long}";
        is_started = true;
      }
      // Region
      if (!(obj_msg->get_scene(0)->get_region().empty())) {
        if (is_started) {
          scene_query = scene_query + ", ";
        }
        scene_query = scene_query + "scn.region = {inp_region}";
        is_started = true;
      }
      // Assets
      if (obj_msg->get_scene(0)->num_assets() > 0) {
        if (is_started) {
          scene_query = scene_query + ", ";
        }
        if (obj_msg->get_op_type() == APPEND) {
          scene_query = scene_query + "scn.assets = coalesce(scn.assets, []) + {inp_assets}";
        } else if (obj_msg->get_op_type() == REMOVE) {
          scene_query = scene_query + "scn.assets = FILTER(asset IN scn.assets WHERE NOT (asset IN {inp_assets}))";
        }
        is_started = true;
      }
      // Tags
      if (obj_msg->get_scene(0)->num_tags() > 0) {
        if (is_started) {
          scene_query = scene_query + ", ";
        }
        if (obj_msg->get_op_type() == APPEND) {
        scene_query = scene_query + "scn.tags = coalesce(scn.tags, []) + {inp_tags}";
        } else if (obj_msg->get_op_type() == REMOVE) {
          scene_query = scene_query + "scn.tags = FILTER(tag IN scn.tags WHERE NOT (tag IN {inp_tags}))";
        }
        is_started = true;
      }

      scene_query = scene_query + " RETURN scn";

      processor_logging->debug("Executing Query:");
      processor_logging->debug(scene_query);

      // Set up the query parameters for scene update
      std::unordered_map<std::string, Neo4jQueryParameterInterface*> \
        scene_params;
      // Key
      std::string qkey = obj_msg->get_scene(0)->get_key();
      key_param = BaseMessageProcessor::get_neo4j_factory()->\
        get_neo4j_query_parameter(qkey);
      processor_logging->debug("Key:");
      processor_logging->debug(qkey);
      scene_params.emplace("inp_key", key_param);
      // Name
      if (!(obj_msg->get_scene(0)->get_name().empty())) {
        std::string qname = obj_msg->get_scene(0)->get_name();
        name_param = BaseMessageProcessor::get_neo4j_factory()->\
          get_neo4j_query_parameter(qname);
        processor_logging->debug("Name:");
        processor_logging->debug(qname);
        scene_params.emplace("inp_name", name_param);
      }
      // Latitude
      if (!(obj_msg->get_scene(0)->get_latitude() == -9999.0)) {
        double qlat = obj_msg->get_scene(0)->get_latitude();
        lat_param = BaseMessageProcessor::get_neo4j_factory()->\
          get_neo4j_query_parameter(qlat);
        scene_params.emplace("inp_lat", lat_param);
      }
      // Longitude
      if (!(obj_msg->get_scene(0)->get_longitude() == -9999.0)) {
        double qlong = obj_msg->get_scene(0)->get_longitude();
        long_param = BaseMessageProcessor::get_neo4j_factory()->\
          get_neo4j_query_parameter(qlong);
        scene_params.emplace("inp_long", long_param);
      }
      // Region
      if (!(obj_msg->get_scene(0)->get_region().empty())) {
        region_param = BaseMessageProcessor::get_neo4j_factory()->\
          get_neo4j_query_parameter(obj_msg->get_scene(0)->get_region());
        processor_logging->debug("Region:");
        processor_logging->debug(obj_msg->get_scene(0)->get_region());
        scene_params.emplace("inp_region", region_param);
      }
      // Assets
      if (obj_msg->get_scene(0)->num_assets() > 0) {
        asset_param = BaseMessageProcessor::get_neo4j_factory()->\
          get_neo4j_query_parameter();
        // Add the assets from the object message to the parameter
        for (int i = 0; i < obj_msg->get_scene(0)->num_assets(); i++) {
          asset_param->add_value(obj_msg->get_scene(0)->get_asset(i));
          processor_logging->debug("Asset:");
          processor_logging->debug(obj_msg->get_scene(0)->get_asset(i));
        }
        scene_params.emplace("inp_assets", asset_param);
      }
      // Tags
      if (obj_msg->get_scene(0)->num_tags() > 0) {
        tag_param = BaseMessageProcessor::get_neo4j_factory()->\
          get_neo4j_query_parameter();
        // Add the assets from the object message to the parameter
        for (int j = 0; j < obj_msg->get_scene(0)->num_tags(); j++) {
          tag_param->add_value(obj_msg->get_scene(0)->get_tag(j));
          processor_logging->debug("Tag:");
          processor_logging->debug(obj_msg->get_scene(0)->get_tag(j));
        }
        scene_params.emplace("inp_tags", tag_param);
      }

      // Execute the query
      ResultTreeInterface *tree = NULL;
      DbObjectInterface* obj = NULL;
      try {
        results = \
          BaseMessageProcessor::get_neo4j_interface()->execute(scene_query, \
          scene_params);
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
        response->set_return_string(obj_msg->get_scene(0)->get_key());
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
    if (asset_param) delete asset_param;
    if (tag_param) delete tag_param;
    if (region_param) delete region_param;

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
ProcessResult* \
  CrudMessageProcessor::process_retrieve_message(SceneListInterface *obj_msg) {
  ProcessResult *response = new ProcessResult;
  if (obj_msg->num_scenes() > 0) {
    ResultsIteratorInterface *results = NULL;
    Neo4jQueryParameterInterface *name_param = NULL;
    Neo4jQueryParameterInterface *lat_param = NULL;
    Neo4jQueryParameterInterface *long_param = NULL;
    Neo4jQueryParameterInterface *key_param = NULL;
    Neo4jQueryParameterInterface *dist_param = NULL;
    Neo4jQueryParameterInterface *region_param = NULL;
    Neo4jQueryParameterInterface *tag_param = NULL;
    processor_logging->debug("Processing Scene Retrieve message");

    if (obj_msg->get_scene(0)->get_name().empty() && \
      obj_msg->get_scene(0)->get_latitude() + 9999.0 < 0.0001 && \
      obj_msg->get_scene(0)->get_longitude() + 9999.0 < 0.0001 && \
      obj_msg->get_scene(0)->get_distance() < 0.0001 && \
      obj_msg->get_scene(0)->get_key().empty() && \
      obj_msg->get_scene(0)->get_region().empty() && \
      obj_msg->get_scene(0)->num_tags() == 0) {
      processor_logging->error("No fields found in get message");
      response->set_error(INSUFF_DATA_ERROR, "Insufficient fields for get");
    } else {
      // Set up the Cypher Query for scene retrieval
      std::string scene_query = "MATCH (scn:Scene";

      // If we have a key present in the request, we can just pull that value
      if (!(obj_msg->get_scene(0)->get_key().empty())) {
        scene_query = scene_query + " {key: {inp_key}})";
      } else {
        // Otherwise, we need to check for other values in the message
        bool inp_name_empty = obj_msg->get_scene(0)->get_name().empty();
        bool inp_region_empty = obj_msg->get_scene(0)->get_region().empty();
        if ((!inp_name_empty) || (!inp_region_empty)) {
          scene_query = scene_query + " {";
          // Start by checking for a name
          if (!inp_name_empty) {
            scene_query = scene_query + "name: {inp_name}";
          }
          if ((!inp_name_empty) && (!inp_region_empty)) {
            scene_query = scene_query + ", ";
          }
          // Check for Region
          if (!inp_region_empty) {
            scene_query = scene_query + "region: {inp_region}";
          }
          scene_query = scene_query + "}";
        }
        scene_query = scene_query + ")";
        bool is_started = false;

        // Check for Tags
        // We will only query for the first tag supplied
        if (obj_msg->get_scene(0)->num_tags() > 0) {
          scene_query = scene_query + " WHERE {inp_tag} in scn.tags";
          is_started = true;
        }

        // Check for a distance-based query
        if (!(obj_msg->get_scene(0)->get_latitude() == -9999.0 || \
          obj_msg->get_scene(0)->get_longitude() == -9999.0 || \
          obj_msg->get_scene(0)->get_distance() < 0.0)) {
          if (is_started) {scene_query = scene_query + " AND";}
          else {scene_query = scene_query + " WHERE";}
          // Query for distance
          // Assumes distance supplied is in meters
          // Haversine formula (https://en.wikipedia.org/wiki/Haversine_formula)
          std::string where_clause = \
                " ( "
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
        key_param = BaseMessageProcessor::get_neo4j_factory()->\
          get_neo4j_query_parameter(qkey);
        processor_logging->debug("Key:");
        processor_logging->debug(qkey);
        scene_params.emplace("inp_key", key_param);
      }
      if (!(obj_msg->get_scene(0)->get_name().empty())) {
        std::string qname = obj_msg->get_scene(0)->get_name();
        name_param = BaseMessageProcessor::get_neo4j_factory()->\
          get_neo4j_query_parameter(qname);
        processor_logging->debug("Name:");
        processor_logging->debug(qname);
        scene_params.emplace("inp_name", name_param);
      }
      if (!(obj_msg->get_scene(0)->get_region().empty())) {
        region_param = BaseMessageProcessor::get_neo4j_factory()->\
          get_neo4j_query_parameter(obj_msg->get_scene(0)->get_region());
        processor_logging->debug("Region:");
        processor_logging->debug(obj_msg->get_scene(0)->get_region());
        scene_params.emplace("inp_region", region_param);
      }
      if (obj_msg->get_scene(0)->num_tags() > 0) {
        tag_param = BaseMessageProcessor::get_neo4j_factory()->\
          get_neo4j_query_parameter(obj_msg->get_scene(0)->get_tag(0));
        processor_logging->debug("Tag:");
        processor_logging->debug(obj_msg->get_scene(0)->get_tag(0));
        scene_params.emplace("inp_tag", tag_param);
      }
      if (obj_msg->get_scene(0)->get_latitude() + 9999.0 > 0.0001 && \
        obj_msg->get_scene(0)->get_longitude() + 9999.0 > 0.0001 && \
        obj_msg->get_scene(0)->get_distance() > 0.0001) {
        double qlat = obj_msg->get_scene(0)->get_latitude();
        lat_param = BaseMessageProcessor::get_neo4j_factory()->\
          get_neo4j_query_parameter(qlat);
        scene_params.emplace("inp_lat", lat_param);
        double qlong = obj_msg->get_scene(0)->get_longitude();
        long_param = BaseMessageProcessor::get_neo4j_factory()->\
          get_neo4j_query_parameter(qlong);
        scene_params.emplace("inp_long", long_param);
        double qdist = obj_msg->get_scene(0)->get_distance();
        dist_param = BaseMessageProcessor::get_neo4j_factory()->\
          get_neo4j_query_parameter(qdist);
        scene_params.emplace("inp_distance", dist_param);
      }

      // Execute the query
      try {
        results = \
          BaseMessageProcessor::get_neo4j_interface()->execute(scene_query, \
          scene_params);
        if (!results) {
          processor_logging->error("No results returned from query");
          response->set_error(PROCESSING_ERROR, "Error processing Scene Get");
        } else {
          // Pull results and return
          SceneListInterface *sc = NULL;
          if (BaseMessageProcessor::get_config_manager()->get_formattype() == \
            PROTO_FORMAT) \
            {sc = BaseMessageProcessor::get_slfactory().build_protobuf_scene();}
          else {sc = BaseMessageProcessor::get_slfactory().build_json_scene();}
          sc->set_err_code(NO_ERROR);
          sc->set_msg_type(SCENE_GET);
          sc->set_transaction_id(obj_msg->get_transaction_id());
          ResultTreeInterface *tree = results->next();
          int num_results = 0;
          while (tree) {
            SceneInterface *data = \
              BaseMessageProcessor::get_sfactory().build_scene();

            // Get the first DB Object (Node)
            DbObjectInterface* obj = tree->get(0);
            processor_logging->debug("Query Result:");
            processor_logging->debug(obj->to_string());

            // Leave the loop if we don't have anything in this result tree
            if (!(obj->is_node()) && !(obj->is_edge())) break;
            num_results = num_results + 1;

            // Pull the node properties and assign them to the new
            // Scene object
            BaseMessageProcessor::get_query_helper()->assign_scene_properties(\
              obj, data);

            sc->add_scene(data);

            // Iterate to the next result
            if (tree) delete tree;
            if (obj) delete obj;
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
          if (sc) delete sc;
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
    if (tag_param) delete tag_param;
    if (region_param) delete region_param;
    return response;
  }
  response->set_error(PROCESSING_ERROR, "No Scene Data recieved");
  return response;
}

// Delete a scene
ProcessResult* \
  CrudMessageProcessor::process_delete_message(SceneListInterface *obj_msg) {
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
      key_param = BaseMessageProcessor::get_neo4j_factory()->\
        get_neo4j_query_parameter(qkey);
      processor_logging->debug("Key:");
      processor_logging->debug(qkey);
      scene_params.emplace("inp_key", key_param);

      // Execute the query
      ResultTreeInterface *tree = NULL;
      DbObjectInterface* obj = NULL;
      try {
        results = \
          BaseMessageProcessor::get_neo4j_interface()->execute(scene_query, \
          scene_params);
      }
      catch (std::exception& e) {
        processor_logging->error("Error running Query:");
        processor_logging->error(e.what());
        response->set_error(PROCESSING_ERROR, e.what());
      }
      if (!results) {
        processor_logging->error("No results returned from delete query");
        response->set_error(PROCESSING_ERROR, "Error processing Scene Delete");
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

ProcessResult* \
    CrudMessageProcessor::process_device_get_message(SceneListInterface *msg) {
  ProcessResult *response = new ProcessResult;
  if (msg->num_scenes() > 0) {
    processor_logging->debug("Processing Device Retrieval message");
    ResultsIteratorInterface *results = NULL;
    Neo4jQueryParameterInterface *key_param = NULL;

    if (msg->get_scene(0)->num_devices() < 1) {
      processor_logging->error("No fields found in device retreival message");
      response->set_error(INSUFF_DATA_ERROR, "Insufficient fields in message");
    } else {
      // Set up the Cypher Query for device retrieval
      std::string scene_query = "MATCH (ud:UserDevice {key: {inp_key}}) RETURN ud";
      processor_logging->debug("Executing Retreival Query");

      // Set up the query parameters for scene delete
      std::unordered_map<std::string, Neo4jQueryParameterInterface*> \
        scene_params;
      std::string device_key = msg->get_scene(0)->get_device(0)->get_key();
      key_param = BaseMessageProcessor::get_neo4j_factory()->\
        get_neo4j_query_parameter(device_key);
      processor_logging->debug("Key:");
      processor_logging->debug(device_key);
      scene_params.emplace("inp_key", key_param);

      // Execute the query
      ResultTreeInterface *tree = NULL;
      DbObjectInterface* obj = NULL;
      try {
        results = \
          BaseMessageProcessor::get_neo4j_interface()->execute(scene_query, \
          scene_params);
      }
      catch (std::exception& e) {
        processor_logging->error("Error running Query:");
        processor_logging->error(e.what());
        response->set_error(PROCESSING_ERROR, e.what());
      }
      if (!results) {
        processor_logging->error("No results returned from device get query");
        response->set_error(PROCESSING_ERROR, "Error processing Device Get");
      } else {
        // Build the response scene list
        SceneListInterface *sc = NULL;
        if (BaseMessageProcessor::get_config_manager()->get_formattype() == \
          PROTO_FORMAT) \
          {sc = BaseMessageProcessor::get_slfactory().build_protobuf_scene();}
        else {sc = BaseMessageProcessor::get_slfactory().build_json_scene();}
        sc->set_err_code(NO_ERROR);
        sc->set_msg_type(DEVICE_GET);
        sc->set_transaction_id(msg->get_transaction_id());
        SceneInterface *empty_scene = BaseMessageProcessor::get_sfactory().build_scene();

        // Process the response
        tree = results->next();
        if (tree) {
          obj = tree->get(0);
          if (obj) {
            processor_logging->debug("Query Result:");
            processor_logging->debug(obj->to_string());

            // Log an error if we don't have a node in this result tree
            if (!(obj->is_node())) {
              processor_logging->error("Non-node returned from query");
              response->set_error(NOT_FOUND, "Unable to find Device");
            } else {
              UserDeviceInterface *data = \
                BaseMessageProcessor::get_udfactory().build_device(device_key);

              // Pull the node properties and assign them to the new
              // Scene object
              BaseMessageProcessor::get_query_helper()->assign_device_properties(\
                obj, data);

              empty_scene->add_device(data);
            }
            sc->add_scene(empty_scene);

            // Set our return string
            processor_logging->debug("Response Scene List:");
            sc->print();
            std::string msg_string;
            sc->to_msg_string(msg_string);
            response->set_return_string(msg_string);

            delete obj;
          } else {
            processor_logging->error("No results returned from Query");
            response->set_error(NOT_FOUND, "Unable to find Device");
          }
          delete tree;
        } else {
          processor_logging->error("No results returned from Query");
          response->set_error(NOT_FOUND, "Unable to find Device");
        }
        if (sc) delete sc;
      }
    }
    if (results) delete results;
    if (key_param) delete key_param;
    return response;
  }
  response->set_error(PROCESSING_ERROR, "No Device Data recieved");
  return response;
}
