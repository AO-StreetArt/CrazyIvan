#include "message_processor.h"

//----------------------------------------------------------------------------//
//---------------------------Internal Methods---------------------------------//
//----------------------------------------------------------------------------//

std::string MessageProcessor::create_uuid() {
  UuidContainer id_container;
  id_container = ugen->generate();
  if (!id_container.err.empty()) {
    uuid_logging->error(id_container.err);
  }
  return id_container.id;
}

void MessageProcessor::get_mutex_lock(std::string obj_key) {
  std::string key = "IVAN-" + obj_key;
  std::string val = "IVAN-" + config->get_nodeid();
  redis_locks->get_lock(key, val);
}

void MessageProcessor::release_mutex_lock(std::string obj_key) {
  std::string key = "IVAN-" + obj_key;
  std::string val = "IVAN-" + config->get_nodeid();
  redis_locks->release_lock(key, val);
}

//----------------------------------------------------------------------------//
//------------------------------Ping & Kill-----------------------------------//
//----------------------------------------------------------------------------//

//Ping the Crazy Ivan Instance
ProcessResult* MessageProcessor::process_ping_message(Scene *obj_msg) {
  processor_logging->debug("Ping Pong");
  return new ProcessResult;
}

//Kill the Crazy Ivan Instance
ProcessResult* MessageProcessor::process_kill_message(Scene *obj_msg) {
  return new ProcessResult;
}

//----------------------------------------------------------------------------//
//-----------------------------CRUD Support-----------------------------------//
//----------------------------------------------------------------------------//

//Create a new scene
ProcessResult* MessageProcessor::process_create_message(Scene *obj_msg) {
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
    UuidContainer id_container;

    //Set up the Cypher Query for scene creation
    std::string scene_query = "CREATE (scn:Scene {key: {inp_key}";

    //Set up the query parameters for scene creation
    std::unordered_map<std::string, Neo4jQueryParameterInterface*> scene_params;
    if (!(obj_msg->get_scene(0)->get_key().empty())) {
      //Use an existing key from the input message
      key_param = neo_factory->get_neo4j_query_parameter(obj_msg->get_scene(0)->get_key());
    }
    else {
      //Generate a new key
      id_container = ugen->generate();
      if (!id_container.err.empty()) {
        uuid_logging->error(id_container.err);
      }
      if (id_container.id.empty()) {
        processor_logging->error("Unknown error generating new key for scene");
        response->set_error(PROCESSING_ERROR, "Unknown error generating new key for scene");
        return response;
      }
      key_param = neo_factory->get_neo4j_query_parameter(id_container.id);
    }
    processor_logging->info("Key:");
    processor_logging->info(id_container.id);
    scene_params.emplace("inp_key", key_param);
    std::string qname = obj_msg->get_scene(0)->get_name();
    if ( !(qname.empty()) ) {
      name_param = neo_factory->get_neo4j_query_parameter(qname);
      processor_logging->debug("Name:");
      processor_logging->debug(qname);
      scene_params.emplace("inp_name", name_param);
      scene_query = scene_query + ", name: {inp_name}";
    }
    if ( !(obj_msg->get_scene(0)->get_latitude() == -9999.0) ) {
      qlat = obj_msg->get_scene(0)->get_latitude();
      lat_param = neo_factory->get_neo4j_query_parameter(qlat);
      processor_logging->debug("Latitude:");
      processor_logging->debug(std::to_string(qlat));
      scene_params.emplace("inp_lat", lat_param);
      scene_query = scene_query + ", latitude: {inp_lat}";
    }
    if ( !(obj_msg->get_scene(0)->get_longitude() == -9999.0) ) {
      qlong = obj_msg->get_scene(0)->get_longitude();
      long_param = neo_factory->get_neo4j_query_parameter(qlong);
      processor_logging->debug("Longitude:");
      processor_logging->debug(std::to_string(qlong));
      scene_params.emplace("inp_long", long_param);
      scene_query = scene_query + ", longitude: {inp_long}";
    }

    scene_query = scene_query + "}) RETURN scn";
    processor_logging->debug(scene_query);

    //Execute the query
    try {
      results = n->execute(scene_query, scene_params);
    }
    catch (std::exception& e) {
      processor_logging->error("Error running Query:");
      processor_logging->error(e.what());
      response->set_error(PROCESSING_ERROR, e.what());
    }

    if (!results) {
      processor_logging->error("No results returned from create query");
      response->set_error(PROCESSING_ERROR, "Unknown Error processing Scene Update");
    }
    else {
      processor_logging->info("Scene Successfully added");
      response->set_return_string(id_container.id);
      tree = results->next();
      if (tree) {
        obj = tree->get(0);
        if (obj) {
          if ( !(obj->is_node()) ) response->set_error(NOT_FOUND, "Document not found");
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

//Update the details of a scene entry
ProcessResult* MessageProcessor::process_update_message(Scene *obj_msg) {

  ProcessResult *response = new ProcessResult;

  if (obj_msg->num_scenes() > 0) {

    //Retrieve a Redis Mutex Lock on the scene
    if (config->get_atomictransactions()) {
      get_mutex_lock(obj_msg->get_scene(0)->get_key());
    }

    //Declare base variables
    bool is_started = false;
    ResultsIteratorInterface *results = NULL;
    Neo4jQueryParameterInterface *name_param = NULL;
    Neo4jQueryParameterInterface *lat_param = NULL;
    Neo4jQueryParameterInterface *long_param = NULL;
    Neo4jQueryParameterInterface *key_param = NULL;
    processor_logging->debug("Processing Scene Update message");

    //Ensure that we have fields in the query
    if ((obj_msg->get_scene(0)->get_name().empty() && obj_msg->get_scene(0)->get_latitude() == -9999.0 && obj_msg->get_scene(0)->get_longitude() == -9999.0) || obj_msg->get_scene(0)->get_key().empty()) {
      processor_logging->error("No fields found in update message");
      response->set_error(INSUFF_DATA_ERROR, "Insufficient fields in message to make update");
    }
    else {
      //Set up the Cypher Query for scene update
      std::string scene_query = "MATCH (scn:Scene {key: {inp_key}}) SET ";
      if ( !(obj_msg->get_scene(0)->get_name().empty()) ) {
        scene_query = scene_query + "scn.name = {inp_name}";
        is_started = true;
      }

      if ( !(obj_msg->get_scene(0)->get_latitude() == -9999.0) ) {
        if (is_started) {
          scene_query = scene_query + ", ";
        }
        scene_query = scene_query + "scn.latitude = {inp_lat}";
        is_started = true;
      }

      if ( !(obj_msg->get_scene(0)->get_longitude() == -9999.0) ) {
        if (is_started) {
          scene_query = scene_query + ", ";
        }
        scene_query = scene_query + "scn.longitude = {inp_long}";
        is_started = true;
      }

      scene_query = scene_query + " RETURN scn";

      processor_logging->debug("Executing Query:");
      processor_logging->debug(scene_query);

      //Set up the query parameters for scene update
      std::unordered_map<std::string, Neo4jQueryParameterInterface*> scene_params;
      std::string qkey = obj_msg->get_scene(0)->get_key();
      key_param = neo_factory->get_neo4j_query_parameter(qkey);
      processor_logging->debug("Key:");
      processor_logging->debug(qkey);
      scene_params.emplace("inp_key", key_param);
      if ( !(obj_msg->get_scene(0)->get_name().empty()) ) {
        std::string qname = obj_msg->get_scene(0)->get_name();
        name_param = neo_factory->get_neo4j_query_parameter(qname);
        processor_logging->debug("Name:");
        processor_logging->debug(qname);
        scene_params.emplace("inp_name", name_param);
      }
      if ( !(obj_msg->get_scene(0)->get_latitude() == -9999.0) ) {
        double qlat = obj_msg->get_scene(0)->get_latitude();
        lat_param = neo_factory->get_neo4j_query_parameter(qlat);
        scene_params.emplace("inp_lat", lat_param);
      }
      if ( !(obj_msg->get_scene(0)->get_longitude() == -9999.0) ) {
        double qlong = obj_msg->get_scene(0)->get_longitude();
        long_param = neo_factory->get_neo4j_query_parameter(qlong);
        scene_params.emplace("inp_long", long_param);
      }

      //Execute the query
      ResultTreeInterface *tree = NULL;
      DbObjectInterface* obj = NULL;
      try {
        results = n->execute(scene_query, scene_params);
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
        response->set_error(PROCESSING_ERROR, "Unknown Error processing Scene Retrieval");
      }
      else {
        response->set_return_string(qkey);
        tree = results->next();
        if (tree) {
          obj = tree->get(0);
          if (obj) {
            if ( !(obj->is_node()) ) response->set_error(NOT_FOUND, "Document not found");
            delete obj;
          }
          delete tree;
        }
      }
    }

    //Cleanup
    if (results) delete results;
    if (name_param) delete name_param;
    if (lat_param) delete lat_param;
    if (long_param) delete long_param;
    if (key_param) delete key_param;

    //Release the Redis Mutex Lock
    if (config->get_atomictransactions()) {
      release_mutex_lock(obj_msg->get_scene(0)->get_key());
    }
    return response;
  }

  response->set_error(PROCESSING_ERROR, "No Scene Data recieved");
  return response;
}

//Query for scene data
ProcessResult* MessageProcessor::process_retrieve_message(Scene *obj_msg) {
  ProcessResult *response = new ProcessResult;
  if (obj_msg->num_scenes() > 0) {
    ResultsIteratorInterface *results = NULL;
    Neo4jQueryParameterInterface *name_param = NULL;
    Neo4jQueryParameterInterface *lat_param = NULL;
    Neo4jQueryParameterInterface *long_param = NULL;
    Neo4jQueryParameterInterface *key_param = NULL;
    Neo4jQueryParameterInterface *dist_param = NULL;
    processor_logging->debug("Processing Scene Retrieve message");

    if (obj_msg->get_scene(0)->get_name().empty() && obj_msg->get_scene(0)->get_latitude() == -9999.0 && obj_msg->get_scene(0)->get_longitude() == -9999.0 && obj_msg->get_scene(0)->get_key().empty()) {
      processor_logging->error("No fields found in get message");
      response->set_error(INSUFF_DATA_ERROR, "Insufficient fields in message to make update");
    }
    else {
      //Set up the Cypher Query for scene retrieval
      std::string scene_query = "MATCH (scn:Scene";

      if ( !(obj_msg->get_scene(0)->get_key().empty()) ) {
        scene_query = scene_query + " {key: {inp_key}})";
      }
      else {
        if ( !(obj_msg->get_scene(0)->get_name().empty()) ) {

          scene_query = scene_query + " {name: {inp_name}}";
        }

        scene_query = scene_query + ")";

        if ( !(obj_msg->get_scene(0)->get_latitude() == -9999.0 || obj_msg->get_scene(0)->get_longitude() == -9999.0 || obj_msg->get_scene(0)->get_distance() < 0.0 ) ) {
          //Query for distance
          //Assumes distance supplied is in meters
          //Haversine formula (https://en.wikipedia.org/wiki/Haversine_formula)
          std::string where_clause = " WHERE ( "
                                        "12742000 * asin("
                                          "sqrt("
                                            "haversin(radians({inp_lat} - scn.latitude)) + "
                                            "cos(radians({inp_lat})) * cos(radians(scn.latitude)) * haversin(radians(scn.longitude - {inp_long}))"
                                          ")"
                                        ")"
                                      ") < {inp_distance}";
          scene_query = scene_query + where_clause;
        }
      }

      scene_query = scene_query + " RETURN scn";

      processor_logging->debug("Executing Query:");
      processor_logging->debug(scene_query);

      //Set up the query parameters for scene retrieval
      std::unordered_map<std::string, Neo4jQueryParameterInterface*> scene_params;
      if ( !(obj_msg->get_scene(0)->get_key().empty()) ) {
        std::string qkey = obj_msg->get_scene(0)->get_key();
        key_param = neo_factory->get_neo4j_query_parameter(qkey);
        processor_logging->debug("Key:");
        processor_logging->debug(qkey);
        scene_params.emplace("inp_key", key_param);
      }
      if ( !(obj_msg->get_scene(0)->get_name().empty()) ) {
        std::string qname = obj_msg->get_scene(0)->get_name();
        name_param = neo_factory->get_neo4j_query_parameter(qname);
        processor_logging->debug("Name:");
        processor_logging->debug(qname);
        scene_params.emplace("inp_name", name_param);
      }
      if ( !(obj_msg->get_scene(0)->get_latitude() == -9999.0 || obj_msg->get_scene(0)->get_longitude() == -9999.0 || obj_msg->get_scene(0)->get_distance() < 0.0) ) {
        double qlat = obj_msg->get_scene(0)->get_latitude();
        lat_param = neo_factory->get_neo4j_query_parameter(qlat);
        scene_params.emplace("inp_lat", lat_param);
        double qlong = obj_msg->get_scene(0)->get_longitude();
        long_param = neo_factory->get_neo4j_query_parameter(qlong);
        scene_params.emplace("inp_long", long_param);
        double qdist = obj_msg->get_scene(0)->get_distance();
        dist_param = neo_factory->get_neo4j_query_parameter(qdist);
        scene_params.emplace("inp_distance", dist_param);
      }

      //Execute the query
      try {
        results = n->execute(scene_query, scene_params);
        if (!results) {
          processor_logging->error("No results returned from query");
          response->set_error(PROCESSING_ERROR, "Unknown Error processing Scene Retrieval");
        }
        else {
          //Pull results and return
          Scene sc;
          sc.set_err_code(NO_ERROR);
          sc.set_msg_type(SCENE_GET);
          sc.set_transaction_id(obj_msg->get_transaction_id());
          ResultTreeInterface *tree = results->next();
          int num_results = 0;
          while (tree) {

            SceneData *data = new SceneData;

            //Get the first DB Object (Node)
            DbObjectInterface* obj = tree->get(0);
            processor_logging->debug("Query Result:");
            processor_logging->debug(obj->to_string());

            //Leave the loop if we don't have anything in this result tree
            if ( !(obj->is_node()) && !(obj->is_edge()) ) break;
            num_results=num_results+1;

            //Pull the node properties and assign them to the new
            //Scene object
            DbMapInterface* map = obj->properties();
            if (map->element_exists("key")) {
              std::string new_key = map->get_string_element("key", 512);
              processor_logging->debug("Key retrieved from query:");
              processor_logging->debug(new_key);
              data->set_key( new_key );
            }
            if (map->element_exists("name")) {
              data->set_name( map->get_string_element("name", 512) );
            }
            if (map->element_exists("latitude")) {
              data->set_latitude( map->get_float_element("latitude") );
            }
            if (map->element_exists("longitude")) {
              data->set_longitude( map->get_float_element("longitude") );
            }

            sc.add_scene(data);

            //Iterate to the next result
            if (tree) delete tree;
            if (obj) delete obj;
            if (map) delete map;
            tree = results->next();
          }
          if (num_results>0) {
            processor_logging->debug("Response Scene List:");
            sc.print();
            if (config->get_formattype() == PROTO_FORMAT) {
              response->set_return_string(sc.to_protobuf());
            }
            else if (config->get_formattype() == JSON_FORMAT) {
              response->set_return_string(sc.to_json());
            }
          }
          else {
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

//Delete a scene
ProcessResult* MessageProcessor::process_delete_message(Scene *obj_msg) {
  ProcessResult *response = new ProcessResult;
  if (obj_msg->num_scenes() > 0) {
    processor_logging->debug("Processing Scene Delete message");
    ResultsIteratorInterface *results = NULL;
    Neo4jQueryParameterInterface *key_param = NULL;

    if ( obj_msg->get_scene(0)->get_key().empty() ) {
      processor_logging->error("No fields found in delete message");
      response->set_error(INSUFF_DATA_ERROR, "Insufficient fields in message to make update");
    }
    else {
      //Set up the Cypher Query for scene delete
      std::string scene_query = "MATCH (scn:Scene {key: {inp_key}}) DETACH DELETE scn RETURN scn";
      processor_logging->debug("Executing Delete Query");

      //Set up the query parameters for scene delete
      std::unordered_map<std::string, Neo4jQueryParameterInterface*> scene_params;
      std::string qkey = obj_msg->get_scene(0)->get_key();
      key_param = neo_factory->get_neo4j_query_parameter(qkey);
      processor_logging->debug("Key:");
      processor_logging->debug(qkey);
      scene_params.emplace("inp_key", key_param);

      //Execute the query
      ResultTreeInterface *tree = NULL;
      DbObjectInterface* obj = NULL;
      try {
        results = n->execute(scene_query, scene_params);
      }
      catch (std::exception& e) {
        processor_logging->error("Error running Query:");
        processor_logging->error(e.what());
        response->set_error(PROCESSING_ERROR, e.what());
      }
      if (!results) {
        processor_logging->error("No results returned from delete query");
        response->set_error(PROCESSING_ERROR, "Unknown Error processing Scene Retrieval");
      }
      else {
        response->set_return_string(qkey);
        tree = results->next();
        if (tree) {
          obj = tree->get(0);
          if (obj) {
            if ( !(obj->is_node()) ) response->set_error(NOT_FOUND, "Document not found");
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

//----------------------------------------------------------------------------//
//-----------------------Strings & Serialization------------------------------//

void MessageProcessor::build_response_scene(int msg_type, int err_code, std::string err_msg, std::string tran_id, std::string scene_id) {
  if (resp_scn) {delete resp_scn;resp_scn=NULL;}
  resp_scn = new Scene;
  //Set up the scene list
  resp_scn->set_msg_type(msg_type);
  resp_scn->set_err_code(err_code);
  resp_scn->set_err_msg(err_msg);
  resp_scn->set_transaction_id(tran_id);
  //Set up the scene
  SceneData *data1 = new SceneData;
  data1->set_key(scene_id);
  resp_scn->add_scene(data1);
}

//Build a protocol buffer serialized string
void MessageProcessor::build_string_response(int msg_type, int err_code, std::string err_msg, std::string tran_id, std::string scene_id, int msg_format_type) {
  build_response_scene(msg_type, err_code, err_msg, tran_id, scene_id);
  //store the serialized string
  if (msg_format_type == PROTO_FORMAT) {
    proto_resp = resp_scn->to_protobuf();
  }
  else if (msg_format_type == JSON_FORMAT) {
    proto_resp = resp_scn->to_json();
  }
}

//Build a protocol buffer serialized string
void MessageProcessor::build_string_response(int msg_type, int err_code, std::string err_msg, std::string tran_id, std::string scene_id, std::string dev_id, Transform &t, int msg_format_type) {
  build_response_scene(msg_type, err_code, err_msg, tran_id, scene_id);
  //Set up the user device
  UserDevice *ud1 = new UserDevice;
  //Add the transformation and key data to the user device
  ud1->set_key(dev_id);
  ud1->set_transform(&t);
  //Add the user device to the scene data
  resp_scn->get_scene(0)->add_device(ud1);
  //store the serialized string
  if (msg_format_type == PROTO_FORMAT) {
    proto_resp = resp_scn->to_protobuf();
  }
  else if (msg_format_type == JSON_FORMAT) {
    proto_resp = resp_scn->to_json();
  }
}

//----------------------------------------------------------------------------//
//--------------------------Device Registration-------------------------------//
//Register a device to a scene
//Predict a coordinate transform for the device
//and pass it back in the response
ProcessResult* MessageProcessor::process_registration_message(Scene *obj_msg) {

  processor_logging->debug("Processing Registration Message");
  //Is this the first device being registered to the scene
  bool is_first_device = false;
  //Does the scene exist in the DB?
  bool does_scene_exist = true;
  //Is the device already registered to the scene
  bool already_registered = false;
  //Is the device already registered to another scene?
  bool previously_registered = false;
  //Current error information
  int current_err_code = NO_ERROR;
  std::string current_err_msg = "";

  //Start by getting a mutex lock against the scene
  processor_logging->debug("Getting mutex lock");
  if (config->get_atomictransactions()) {
    try {
      get_mutex_lock(obj_msg->get_scene(0)->get_key());
    }
    catch (std::exception& e) {
      processor_logging->error("Error getting mutex lock");
      processor_logging->error(e.what());
      current_err_code = PROCESSING_ERROR;
      current_err_msg = e.what();
    }
  }

  //Determine if the scene exists in the DB
  if (current_err_code == NO_ERROR) {
    try {
      does_scene_exist = qh->scene_exists(obj_msg->get_scene(0)->get_key());
    }
    catch (std::exception& e) {
      processor_logging->error("Error checking for scene existence");
      processor_logging->error(e.what());
      current_err_code = PROCESSING_ERROR;
      current_err_msg = e.what();
    }
  }

  //Determine if this is the first device being registered to the scene
  if (current_err_code == NO_ERROR) {
    if ( !(does_scene_exist) ) {
      is_first_device = true;
      //Create the scene
      processor_logging->debug("Creating Scene in database");
      ProcessResult *response = process_create_message(obj_msg);
      if ( !(response->successful()) ) {
        processor_logging->error("Scene not found and creation failed");
        current_err_code = PROCESSING_ERROR;
        current_err_msg = "Scene not found and creation failed";
      }
      delete response;
    }
    else {
      try {
        already_registered = qh->is_ud_registered(obj_msg->get_scene(0)->get_key(),\
          obj_msg->get_scene(0)->get_device(0)->get_key());
      }
      catch (std::exception& e) {
        processor_logging->error("Error checking if User Device is already registered");
        processor_logging->error(e.what());
        current_err_code = PROCESSING_ERROR;
        current_err_msg = e.what();
      }
    }
  }

  //Find other scenes that the object is registered to
  Scene *registered_scenes = NULL;
  if (current_err_code == NO_ERROR) {
    try {
      registered_scenes = qh->get_registrations(obj_msg->get_scene(0)->get_device(0)->get_key());
    }
    catch (std::exception& e) {
      processor_logging->error("Error getting registrations");
      processor_logging->error(e.what());
      current_err_code = PROCESSING_ERROR;
      current_err_msg = e.what();
    }
  }
  if (!registered_scenes) {
    previously_registered = false;
  }

  //The transform matrix calculated for the given device
  //If we are registering the first device or cannot find
  //any paths from the device's previous scene(s), then we will
  //leave this as the identity matrix
  Transform new_transform;

  //If we are not registering the first device, the scene exists,
  //the device is not already registered to the scene in question but
  //is registered to other scenes
  if ( !(is_first_device) && does_scene_exist && !(already_registered) && current_err_code == NO_ERROR && previously_registered ) {
    processor_logging->debug("Attempting to calculate UD Transform from Existing Registrations");
    //Iterate through the scenes the device is already registered to
    //Try to find a path from these scenes to the current one in order
    //to establish a transform
    for (int i = 0; i < registered_scenes->num_scenes(); i++) {

      try {
        SceneTransformResult st_result = qh->calculate_scene_scene_transform(\
          registered_scenes->get_scene(i)->get_key(), obj_msg->get_scene(0)->get_key());
        if (st_result.result_flag) {
          new_transform.add_transform(st_result.transform);
        }
      }
      catch (std::exception& e) {
        processor_logging->error("Error calculating Scene-Scene Transform");
        processor_logging->error(e.what());
        current_err_code = PROCESSING_ERROR;
        current_err_msg = e.what();
      }
    }
  }

  if ( !(already_registered) && current_err_code == NO_ERROR ) {
    //Create the registration link in the DB
    processor_logging->debug("Registering Device in the DB");
    try {
      qh->register_device_to_scene(obj_msg->get_scene(0)->get_device(0)->get_key(),\
        obj_msg->get_scene(0)->get_key(), new_transform);
    }
    catch (std::exception& e) {
      processor_logging->error("Error Registering Device");
      processor_logging->error(e.what());
      current_err_code = PROCESSING_ERROR;
      current_err_msg = e.what();
    }
  }

  //If we are registering to another scene and that scene is new, then
  //we can put in place a coordinate system transformation equal to the device
  //transform from the old scene, as the transform from the new scene is the identity matrix
  if (previously_registered && is_first_device && current_err_code == NO_ERROR) {
    qh->process_UDUD_transformation(registered_scenes, obj_msg);
  }

  //Build a protocol buffer response
  processor_logging->debug("Creating Response message");
  build_string_response(SCENE_ENTER, current_err_code,\
    current_err_msg, obj_msg->get_transaction_id(), obj_msg->get_scene(0)->get_key(), config->get_formattype());

  if (registered_scenes) {
    delete registered_scenes;
  }

  if (config->get_atomictransactions()) {
    release_mutex_lock(obj_msg->get_scene(0)->get_key());
  }
  return new ProcessResult(proto_resp);
}

//Remove a device from a scene
//If we are removing the last user device from a scene,
//then remove the scene and try to move any paths that pass through the scene
ProcessResult* MessageProcessor::process_deregistration_message(Scene *obj_msg) {

  processor_logging->debug("Processing Deregistration Message");
  //Current error information
  int current_err_code = NO_ERROR;
  std::string current_err_msg = "";

  //Start by getting a mutex lock against the scene
  processor_logging->debug("Getting mutex lock");
  try {
    get_mutex_lock(obj_msg->get_scene(0)->get_key());
  }
  catch (std::exception& e) {
    processor_logging->error("Error Getting Mutex Lock");
    processor_logging->error(e.what());
    current_err_code = PROCESSING_ERROR;
    current_err_msg = e.what();
  }

  //Remove the User Device
  if (current_err_code == NO_ERROR) {
    processor_logging->debug("Removing Device from Scene");
    try {
      qh->remove_device_from_scene(obj_msg->get_scene(0)->get_device(0)->get_key(),\
        obj_msg->get_scene(0)->get_key());
    }
    catch (std::exception& e) {
      processor_logging->error("Error Removing Device From Scene");
      processor_logging->error(e.what());
      current_err_code = PROCESSING_ERROR;
      current_err_msg = e.what();
    }
  }
  //Build a protocol buffer response
  processor_logging->debug("Creating Response message");
  build_string_response(SCENE_LEAVE, current_err_code,\
    current_err_msg, obj_msg->get_transaction_id(), obj_msg->get_scene(0)->get_key(), config->get_formattype());

  release_mutex_lock(obj_msg->get_scene(0)->get_key());
  return new ProcessResult(proto_resp);
}

//----------------------------------------------------------------------------//
//----------------------------Device Alignment--------------------------------//

//Align an objects transformation with a scene
//If the object is a member of other scenes:
//  -If a coordinate system transformation doesn't exist, then create one
//  -If a coordinate system transformation exists, then update it with a correction
ProcessResult* MessageProcessor::process_device_alignment_message(Scene *obj_msg) {
  processor_logging->debug("Processing Alignment Message");
  //Current error information
  int current_err_code = NO_ERROR;
  std::string current_err_msg = "";

  //Start by getting a mutex lock against the scene
  processor_logging->debug("Getting mutex lock");
  try {
    get_mutex_lock(obj_msg->get_scene(0)->get_key());
  }
  catch (std::exception& e) {
    processor_logging->error("Error Getting Mutex Lock");
    processor_logging->error(e.what());
    current_err_code = PROCESSING_ERROR;
    current_err_msg = e.what();
  }

  //Update the transformation between the scene and user device
  if (current_err_code == NO_ERROR) {
    try {
      qh->update_device_registration( obj_msg->get_scene(0)->get_device(0)->get_key(),\
        obj_msg->get_scene(0)->get_key(), *(obj_msg->get_scene(0)->get_device(0)->get_transform()) );
    }
    catch (std::exception& e) {
      processor_logging->error("Error Updating Device Registration");
      processor_logging->error(e.what());
      current_err_code = PROCESSING_ERROR;
      current_err_msg = e.what();
    }
  }

  //Find scenes that the object is registered to
  Scene *registered_scenes = NULL;
  if (current_err_code == NO_ERROR) {
    processor_logging->debug("Retrieving Scenes already registered to");
    try {
      registered_scenes = qh->get_registrations(obj_msg->get_scene(0)->get_device(0)->get_key());
    }
    catch (std::exception& e) {
      processor_logging->error("Error Retrieving Device Registrations");
      processor_logging->error(e.what());
      current_err_code = PROCESSING_ERROR;
      current_err_msg = e.what();
    }
  }

  //Iterate through the scenes the devices is registered with &
  //Correct/Create scene-scene transforms
  Transform new_transform;
  if (current_err_code == NO_ERROR) {
    processor_logging->debug("Updating Scene-Scene Transforms");
    try {
      qh->process_UDUD_transformation(registered_scenes, obj_msg);
    }
    catch (std::exception& e) {
      processor_logging->error("Error Correcting/Creating Scene-Scene Transforms");
      processor_logging->error(e.what());
      current_err_code = PROCESSING_ERROR;
      current_err_msg = e.what();
    }
  }

  //Build a protocol buffer response
  processor_logging->debug("Creating Response message");
  build_string_response(DEVICE_ALIGN, current_err_code,\
    current_err_msg, obj_msg->get_transaction_id(), obj_msg->get_scene(0)->get_key(), config->get_formattype());

  if (registered_scenes) {
    delete registered_scenes;
  }

  release_mutex_lock(obj_msg->get_scene(0)->get_key());
  return new ProcessResult(proto_resp);
}
