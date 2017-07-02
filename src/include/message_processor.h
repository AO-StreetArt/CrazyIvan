#include "ivan_log.h"
#include "ivan_utils.h"
#include "redis_locking.h"
#include "configuration_manager.h"
#include "scene.h"
#include "query_helper.h"

#include "rapidjson/document.h"

#include "aossl/neo4j/include/neo4j_interface.h"
#include "aossl/redis/include/redis_interface.h"
#include "aossl/uuid/include/uuid_interface.h"
#include "aossl/zmq/include/zmq_interface.h"

#ifndef MSG_PROCESSOR
#define MSG_PROCESSOR

class ProcessResult {
int err_code;
std::string err_desc;
const char * err_cdesc;
std::string return_str;
bool success;
public:
  //Default construct returns a successful process result
  ProcessResult() {err_code=100;err_desc="";success=true;return_str="";}
  ProcessResult(int ecode, std::string edesc) {err_code=ecode;err_desc=edesc;success=false;return_str="";}
  ProcessResult(std::string data) {return_str=data;err_code=100;err_desc="";success=true;}
  void set_error(int ecode, std::string edesc) {err_code=ecode;err_desc=edesc;success=false;return_str="";}
  void set_error(int ecode, const char * edesc) {err_code=ecode;err_cdesc=edesc;err_desc.assign(err_cdesc);success=false;return_str="";}
  int get_error_code() {return err_code;}
  std::string get_error_description() {return err_desc;}
  void set_return_string(std::string key) {return_str=key;}
  std::string get_return_string() {return return_str;}
  bool successful() {return success;}
};

//The class containing core logic for CrazyIvan
//Accepts an Scene (assuming it is an inbound message)
//And performs any and all processing on it,
//Includes Redis Locks, DB Interactions, and any necessary calculations
class MessageProcessor {

//--------------------------Internal Variables--------------------------------//

Neo4jInterface *n = NULL;
Neo4jComponentFactory *neo_factory = NULL;
RedisInterface *r = NULL;
ConfigurationManager *config = NULL;
RedisLocker *redis_locks = NULL;
uuidInterface *ugen = NULL;
QueryHelper *qh = NULL;
std::string proto_resp = "";
Scene *resp_scn = NULL;

//---------------------------Internal Methods---------------------------------//

std::string create_uuid();
void get_mutex_lock(std::string obj_key);
void release_mutex_lock(std::string obj_key);
void build_response_scene(int msg_type, int err_code, std::string err_msg, std::string tran_id, std::string scene_id);
void build_string_response(int msg_type, int err_code, std::string err_msg, std::string tran_id, std::string scene_id, int msg_format_type);
void build_string_response(int msg_type, int err_code, std::string err_msg, std::string tran_id, std::string scene_id, std::string dev_id, Transform &t, int msg_format_type);

//------------------------------Ping & Kill-----------------------------------//

//Ping the Crazy Ivan Instance
ProcessResult* process_ping_message(Scene *obj_msg);

//Kill the Crazy Ivan Instance
ProcessResult* process_kill_message(Scene *obj_msg);

//-----------------------------CRUD Support-----------------------------------//

//Create a new scene
ProcessResult* process_create_message(Scene *obj_msg);

//Update the details of a scene entry
ProcessResult* process_update_message(Scene *obj_msg);

//Query for scene data
ProcessResult* process_retrieve_message(Scene *obj_msg);

//Delete a scene
ProcessResult* process_delete_message(Scene *obj_msg);

//----------------------------------------------------------------------------//
//--------------------------Device Registration-------------------------------//
//Register a device to a scene
//Predict a coordinate transform for the device
//and pass it back in the response
ProcessResult* process_registration_message(Scene *obj_msg);

//Remove a device from a scene
//If we are removing the last user device from a scene,
//then remove the scene and try to move any paths that pass through the scene
ProcessResult* process_deregistration_message(Scene *obj_msg);

//Align an objects transformation with a scene
//If the object is a member of other scenes:
//  -If a coordinate system transformation doesn't exist, then create one
//  -If a coordinate system transformation exists, then update it with a correction
ProcessResult* process_device_alignment_message(Scene *obj_msg);

//----------------------------------------------------------------------------//
//-----------------------------Public Methods---------------------------------//
//----------------------------------------------------------------------------//
public:

  //Constructor
  inline MessageProcessor(Neo4jComponentFactory *nf, Neo4jInterface *neo4j, RedisInterface *rd, ConfigurationManager *con, uuidInterface *u) {
    n=neo4j;
    r = rd;
    config = con;
    redis_locks = new RedisLocker(rd);
    ugen=u;
    neo_factory = nf;
    qh=new QueryHelper(n, neo_factory);
  }

  //Destructor
  ~MessageProcessor() {delete redis_locks;delete qh;if (resp_scn) {delete resp_scn;}}

  //TO-DO: If we wrap this in a method that accepts a const char *
  //TO-DO: and make this threadsafe, then we can turn this into an
  //TO-DO: asynchronous HTTP Service

  //Process a message in the form of an Scene
  //In the case of a get message, return the retrieved document back to the main method
  //In the case of a create message, return the key of the created object back to the main method
  //Otherwise, return a blank string in the case of success, and "-1" in the case of failure
  inline ProcessResult* process_message(Scene *obj_msg) {

    //Determine what type of message we have, and act accordingly
    int msg_type = obj_msg->get_msg_type();
    processor_logging->debug("Message Type: " + std::to_string(msg_type));

    if (msg_type == SCENE_CRT) {
      return process_create_message(obj_msg);
    }
    else if (msg_type == SCENE_UPD) {
      return process_update_message(obj_msg);
    }
    else if (msg_type == SCENE_GET) {
      return process_retrieve_message(obj_msg);
    }
    else if (msg_type == SCENE_DEL) {
      return process_delete_message(obj_msg);
    }
    else if (msg_type == SCENE_ENTER) {
      return process_registration_message(obj_msg);
    }
    else if (msg_type == SCENE_LEAVE) {
      return process_deregistration_message(obj_msg);
    }
    else if (msg_type == DEVICE_ALIGN) {
      return process_device_alignment_message(obj_msg);
    }
    else if (msg_type == KILL) {
      return process_kill_message(obj_msg);
    }
    else if (msg_type == PING) {
      return process_ping_message(obj_msg);
    }
    else {
      return new ProcessResult(BAD_MSG_TYPE_ERROR, "Unrecognized Message Type");
    }
  }

};

#endif