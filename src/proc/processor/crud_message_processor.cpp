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
    ResultTreeInterface *tree = NULL;
    DbObjectInterface* obj = NULL;
    processor_logging->info("Scenes Found in creation message");

    // Empty string to hold a newly generated ID, if needed
    std::string new_id;
    if (obj_msg->get_scene(0)->get_key().empty()) {
      // Generate a new key
      BaseMessageProcessor::create_uuid(new_id);
      if (new_id.empty()) {
        processor_logging->error("Unknown error generating new key for scene");
        response->set_error(PROCESSING_ERROR, "Error generating key for scene");
        return response;
      }
      processor_logging->info("Key Generated:");
      processor_logging->info(new_id);
    } else {
      new_id = obj_msg->get_scene(0)->get_key();
    }
    response->set_return_string(new_id);

    // Set up the Cypher Query for scene creation
    std::string scene_query;
    BaseMessageProcessor::get_query_helper()->generate_scene_crud_query(new_id, \
      CREATE_QUERY_TYPE, APPEND, obj_msg->get_scene(0), scene_query);

    processor_logging->debug("Executing Query:");
    processor_logging->debug(scene_query);

    // Set up the query parameters for scene creation
    std::unordered_map<std::string, Neo4jQueryParameterInterface*> scene_params;
    BaseMessageProcessor::get_query_helper()->generate_scene_query_parameters(new_id, \
      CREATE_QUERY_TYPE, obj_msg->get_scene(0), scene_params);

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

    // Resource cleanup
    if (results) delete results;
    for (std::pair<std::string, Neo4jQueryParameterInterface*> element : scene_params) {
      delete element.second;
    }
    return response;
  }
  response->set_error(PROCESSING_ERROR, "No Scene Data recieved");
  return response;
}

// Update the details of a scene entry
ProcessResult* \
  CrudMessageProcessor::process_update_message(SceneListInterface *obj_msg) {
  ProcessResult *response = new ProcessResult;
  std::unordered_map<std::string, Neo4jQueryParameterInterface*> \
    scene_params;

  if (obj_msg->num_scenes() > 0) {
    // Retrieve a Redis Mutex Lock on the scene
    if (BaseMessageProcessor::get_config_manager()->get_atomictransactions()) {
      get_mutex_lock(obj_msg->get_scene(0)->get_key());
    }

    // Declare base variables
    ResultsIteratorInterface *results = NULL;
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
      std::string scene_query;
      BaseMessageProcessor::get_query_helper()->generate_scene_crud_query(obj_msg->get_scene(0)->get_key(), \
        UPDATE_QUERY_TYPE, obj_msg->get_op_type(), \
        obj_msg->get_scene(0), scene_query);

      processor_logging->debug("Executing Query:");
      processor_logging->debug(scene_query);

      // Set up the query parameters for scene update
      BaseMessageProcessor::get_query_helper()->generate_scene_query_parameters(obj_msg->get_scene(0)->get_key(), \
        UPDATE_QUERY_TYPE, obj_msg->get_scene(0), scene_params);

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
    for (std::pair<std::string, Neo4jQueryParameterInterface*> element : scene_params) {
      delete element.second;
    }

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
  std::unordered_map<std::string, Neo4jQueryParameterInterface*> \
    scene_params;
  if (obj_msg->num_scenes() > 0) {
    ResultsIteratorInterface *results = NULL;
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
      std::string scene_query;
      BaseMessageProcessor::get_query_helper()->generate_scene_crud_query(obj_msg->get_scene(0)->get_key(), \
        GET_QUERY_TYPE, APPEND, obj_msg->get_scene(0), scene_query);

      processor_logging->debug("Executing Query:");
      processor_logging->debug(scene_query);

      // Set up the query parameters for scene retrieval
      BaseMessageProcessor::get_query_helper()->generate_scene_query_parameters(obj_msg->get_scene(0)->get_key(), \
        GET_QUERY_TYPE, obj_msg->get_scene(0), scene_params);

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
    for (std::pair<std::string, Neo4jQueryParameterInterface*> element : scene_params) {
      delete element.second;
    }
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

      // Set up the query parameters for scene retrieval
      std::unordered_map<std::string, Neo4jQueryParameterInterface*> \
        scene_params;
      BaseMessageProcessor::get_query_helper()->generate_scene_query_parameters(obj_msg->get_scene(0)->get_key(), \
        DELETE_QUERY_TYPE, NULL, scene_params);

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
