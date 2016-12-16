#include "ivan_log.h"
#include "ivan_utils.h"
#include "redis_locking.h"
#include "configuration_manager.h"
#include "scene.h"

#include "rapidjson/document.h"

#include "aossl/factory/redis_interface.h"
#include "aossl/factory/neo4j_interface.h"
#include "aossl/factory/zmq_interface.h"

//The class containing core logic for CrazyIvan
//Accepts an Scene (assuming it is an inbound message)
//And performs any and all processing on it,
//Includes Redis Locks, DB Interactions, and OB ZMQ messaging

#ifndef MSG_PROCESSOR
#define MSG_PROCESSOR

class MessageProcessor {

Neo4jInterface *n = NULL;
RedisInterface *r = NULL;
ConfigurationManager *config = NULL;
RedisLocker *redis_locks = NULL;

//Create a new scene
inline std::string process_create_message(Scene *obj_msg) {
  return "Not Implemented";
}

//Update the details of a scene entry
inline std::string process_update_message(Scene *obj_msg) {
  return "Not Implemented";
}

//Query for scene data
inline std::string process_retrieve_message(Scene *obj_msg) {
  return "Not Implemented";
}

//Delete a scene
inline std::string process_delete_message(Scene *obj_msg) {
  return "Not Implemented";
}

//Register a device to a scene
//Predict a coordinate transform for the device
//and pass it back in the response
inline std::string process_registration_message(Scene *obj_msg) {
  return "Not Implemented";
}

//Remove a device from a scene
//If the object is a member of other scenes:
//  -If a coordinate system transformation exists, then update it with a correction
//  -If a coordinate system transformation doesn't exist, then create one
inline std::string process_deregistration_message(Scene *obj_msg) {
  return "Not Implemented";
}

//Align an objects transformation with a scene
inline std::string process_device_alignment_message(Scene *obj_msg) {
  return "Not Implemented";
}

//Ping the Crazy Ivan Instance
inline std::string process_ping_message(Scene *obj_msg) {
  processor_logging->debug("Ping Pong");
  return "";
}

//Kill the Crazy Ivan Instance
inline std::string process_kill_message(Scene *obj_msg) {
  return "";
}
public:

  //Constructor & Destructor
  MessageProcessor(Neo4jInterface *neo4j, RedisInterface *rd, ConfigurationManager *con) {n=neo4j; r = rd;config = con;redis_locks = new RedisLocker(rd);}
  ~MessageProcessor() {delete redis_locks;}

  //Process a message in the form of an Scene
  //In the case of a get message, return the retrieved document back to the main method
  //In the case of a create message, return the key of the created object back to the main method
  //Otherwise, return a blank string in the case of success, and "-1" in the case of failure
  inline std::string process_message(Scene *obj_msg) {

    //Determine what type of message we have, and act accordingly
    int msg_type = obj_msg->get_msg_type();
    processor_logging->debug("Message Type: " + std::to_string(msg_type));
    std::string process_return;

    if (msg_type == SCENE_CRT) {
      process_return = process_create_message(obj_msg);
    }
    else if (msg_type == SCENE_UPD) {
      process_return = process_update_message(obj_msg);
    }
    else if (msg_type == SCENE_GET) {
      process_return = process_retrieve_message(obj_msg);
    }
    else if (msg_type == SCENE_DEL) {
      process_return = process_delete_message(obj_msg);
    }
    else if (msg_type == SCENE_ENTER) {
      process_return = process_registration_message(obj_msg);
    }
    else if (msg_type == SCENE_LEAVE) {
      process_return = process_deregistration_message(obj_msg);
    }
    else if (msg_type == DEVICE_ALIGN) {
      process_return = process_device_alignment_message(obj_msg);
    }
    else if (msg_type == KILL) {
      process_return = process_kill_message(obj_msg);
    }
    else if (msg_type == PING) {
      process_return = process_ping_message(obj_msg);
    }
    else {
      process_return = "-1";
    }

    return process_return;
  }
};

#endif
