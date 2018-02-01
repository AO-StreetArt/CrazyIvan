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

// This sets up all of the components necessary for the service and runs the
// main loop for the application.

#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <cstdlib>
#include <sstream>
#include <string>
#include <cstring>
#include <iostream>
#include <fstream>
#include <exception>
#include <vector>
#include <thread>

#include "boost/asio.hpp"

#include "src/app/include/ivan_log.h"
#include "src/app/include/ivan_utils.h"
#include "src/app/include/configuration_manager.h"
#include "src/app/include/globals.h"

#include "src/api/include/scene_list_interface.h"
#include "src/api/include/scene_list_factory.h"
#include "src/api/include/Scene.pb.h"

#include "src/model/include/scene_interface.h"
#include "src/model/include/scene_factory.h"

#include "src/proc/processor/include/message_processor.h"

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/error/en.h"

#include "cppkafka/consumer.h"
#include "cppkafka/configuration.h"

#include "aossl/commandline/include/commandline_interface.h"
#include "aossl/commandline/include/factory_cli.h"

#include "aossl/neo4j/include/neo4j_interface.h"
#include "aossl/neo4j/include/factory_neo4j.h"

#include "aossl/logging/include/logging_interface.h"
#include "aossl/logging/include/factory_logging.h"

#include "aossl/redis/include/redis_interface.h"
#include "aossl/redis/include/factory_redis.h"

#include "aossl/uuid/include/uuid_interface.h"
#include "aossl/uuid/include/factory_uuid.h"

#include "aossl/zmq/include/zmq_interface.h"
#include "aossl/zmq/include/factory_zmq.h"

bool running = true;
bool finished_running = false;

// Catch a Signal (for example, keyboard interrupt)
void my_signal_handler(int s) {
  main_logging->error("Caught signal");
  running = false;
  std::string signal_type = std::to_string(s);
  main_logging->error(signal_type);
  // Wait for the Kafka thread to finish using the main assets before exit
  while (!finished_running) {}
  shutdown();
  exit(1);
}

// Send messages to User Devices registered to scene specified by input
void send_device_messages(std::string inp_string, std::string inbound_msg, boost::asio::ip::udp::socket &sock, boost::asio::io_service &io_service) {
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
  key_param = \
    neo4j_factory->get_neo4j_query_parameter(qkey);
  main_logging->debug("Key:");
  main_logging->debug(qkey);
  q_params.emplace("inp_key", key_param);

  // Execute the query
  std::string exc_string = "";
  try {
    results = \
      neo->execute(q_string, q_params);
  }
  catch (std::exception& e) {
    main_logging->error("Error running Query:");
    main_logging->error(q_string);
    main_logging->error(e.what());
  }

  if (!results) {
    main_logging->debug("User Device not found registered to scene");
    // If we are registering the first device, then we may have a created scene
  } else {
    // Check if the registering user device is already registered
    tree = results->next();
    while (tree) {
      // Get the first DB Object (Node)
      obj = tree->get(0);
      main_logging->debug("Query Result:");
      main_logging->debug(obj->to_string());

      if (!(obj->is_node())) break;

      // Pull the node properties and assign them to the new
      // Scene object
      map = obj->properties();
      std::string dev_host;
      int dev_port;
      if (map->element_exists("hostname")) {
        dev_host = map->get_string_element("hostname");
      }
      if (map->element_exists("port")) {
        dev_port = map->get_int_element("port");
      }

      // Send a UDP Message to the specified device
      try {
        boost::asio::ip::udp::endpoint remote_endpoint;
        remote_endpoint = boost::asio::ip::udp::endpoint(boost::asio::ip::address::from_string(dev_host), dev_port);
        boost::system::error_code err;
        sock.send_to(boost::asio::buffer(inbound_msg, inbound_msg.size()), remote_endpoint, 0, err);
      } catch (std::exception& e) {
        main_logging->error("Exception while sending UDP Message:");
        main_logging->error(e.what());
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
}

// Start up monitoring on a Kafka Queue for Object Change Streams
void monitor_kafka_queue(std::string kafka_address) {
  main_logging->debug("Starting Kafka Monitoring Thread");
  boost::asio::io_service io_service;
  boost::asio::ip::udp::socket socket(io_service);
  socket.open(boost::asio::ip::udp::v4());
  // Create Consumer config
  cppkafka::Configuration config = {
        { "metadata.broker.list", kafka_address },
        { "group.id", "_dvs_consumers" },
        { "enable.auto.commit", false }
  };
  // Create the consumer
  cppkafka::Consumer consumer(config);

  // Print the assigned partitions on assignment
  consumer.set_assignment_callback([](const cppkafka::TopicPartitionList& partitions) {
      if (main_logging) {
        main_logging->debug("Assigned to Kafka Partition");
      }
  });

  // Print the revoked partitions on revocation
  consumer.set_revocation_callback([](const cppkafka::TopicPartitionList& partitions) {
      if (main_logging) {
        main_logging->debug("Revoked from Kafka Partition");
      }
  });

  // Subscribe to the topic
  consumer.subscribe({ "_dvs" });

  main_logging->info("Subscribed to Kafka Topic _dvs");

  while (running) {
    cppkafka::Message msg = consumer.poll();
    if (msg) {
      // If we managed to get a message
      if (msg.get_error()) {
        // Ignore EOF notifications from rdkafka
        if (!msg.is_eof()) {
          main_logging->debug("Recieved Kafka EOF Error");
          main_logging->debug(msg.get_error().to_string());
        } else {
          main_logging->debug("Recieved Kafka Error");
          main_logging->debug(msg.get_error().to_string());
        }
      } else {
        main_logging->info("Recieved Kafka Message");
        main_logging->debug(msg.get_key());
        main_logging->debug(msg.get_payload());
        std::string inbound_msg = msg.get_payload();
        // Now commit the message
        consumer.commit(msg);
        main_logging->debug(inbound_msg);
        // Parse message
        std::string scene_id;
        try {
          rapidjson::Document d;
          d.Parse<rapidjson::kParseStopWhenDoneFlag>(inbound_msg.c_str());
          if (d.HasParseError()) {
            main_logging->error("Parsing Error: ");
            main_logging->error(GetParseError_En(d.GetParseError()));
          } else {
            // Pull the scene from the document
            if (d.IsObject()) {
              // Message Type
              rapidjson::Value::ConstMemberIterator scene_iter = \
                d.FindMember("scene");
              if (scene_iter != d.MemberEnd()) {
                obj_logging->debug("Scene found");
                if (!(scene_iter->value.IsNull())) {
                  scene_id = scene_iter->value.GetString();
                }
              }
            }
          }
        }
        // Catch a possible error and write to logs
        catch (std::exception& e) {
          main_logging->error("Exception while parsing inbound document:");
          main_logging->error(e.what());
        }
        // Query Neo4j for User Devices registered to the scene of the
        // update, then send UDP message to all (each on their own threads)
        if (!(scene_id.empty())) {
          send_device_messages(scene_id, inbound_msg, socket, io_service);
        }
      }
    }
  }
  socket.close();
  finished_running = true;
}

// -----------------------
// ------Main Method------
// -----------------------

int main(int argc, char** argv) {
  // Set up internal variables
  int current_error_code;
  std::string current_error_message;
  int msg_type = -1;
  protoScene::SceneList new_proto;

  // Set up a handler for any signal events so we always shutdown gracefully
  struct sigaction sigIntHandler;
  sigIntHandler.sa_handler = my_signal_handler;
  sigemptyset(&sigIntHandler.sa_mask);
  sigIntHandler.sa_flags = 0;

  sigaction(SIGINT, &sigIntHandler, NULL);

  cli_factory = new CommandLineInterpreterFactory;
  neo4j_factory = new Neo4jComponentFactory;
  redis_factory = new RedisComponentFactory;
  uuid_factory = new uuidComponentFactory;
  zmq_factory = new ZmqComponentFactory;
  logging_factory = new LoggingComponentFactory;

  scene_list_factory = new SceneListFactory;
  processor_factory = new ProcessorFactory;

  // Set up our command line interpreter
  cli = cli_factory->get_command_line_interface(argc, argv);

  // Allow for wait on startup, if configured
  if (cli->opt_exist("-wait")) {
    std::string wait_time_string = cli->get_opt("-wait");
    int wait_time = std::stoi(wait_time_string, NULL);
    // Accept input on the command line in seconds, convert to microseconds
    usleep(wait_time * 1000000);
  }

  // Set up logging
  std::string initFileName;

  // See if we have a command line setting for the log file
  const char * env_logging_file = std::getenv("CRAZYIVAN_LOGGING_CONF");
  if (env_logging_file) {
    std::string tempFileName(env_logging_file);
    initFileName = tempFileName;
  } else if (cli->opt_exist("-log-conf")) {
    initFileName = cli->get_opt("-log-conf");
  } else {
    initFileName = "log4cpp.properties";
  }

  // This reads the logging configuration file
  logging = logging_factory->get_logging_interface(initFileName);

  // Set up the logging submodules for each category
  start_logging_submodules();

  // Set up the UUID Generator
  ua = uuid_factory->get_uuid_interface();

  std::string service_instance_id = "CrazyIvan";

  // Set up our configuration manager with the CLI
  cm = new ConfigurationManager(cli, service_instance_id);

  // The configuration manager will  look at any command line arguments,
  // configuration files, and Consul connections to try and determine the
  // correct configuration for the service
  bool config_success = false;
  bool config_tried = false;
  // If we fail configuration, we should sleep for 5 seconds and try again
  while (!config_success) {
    if (config_tried) {
      main_logging->error("Configuration Failed, trying again in 5 seconds");
      usleep(5000000);
    } else {
      config_tried = true;
    }
    try {
      config_success = cm->configure();
    }
    catch (std::exception& e) {
      main_logging->error("Exception encountered during Configuration");
      shutdown();
      exit(1);
    }
  }

  // Set up our Redis Connection List, which is passed to the Redis Admin
  // We just pass the first element until cluster support is added
  std::vector<RedisConnChain> RedisConnectionList = cm->get_redisconnlist();
  // Set up Redis Connection
  if (RedisConnectionList.size() > 0) {
    try {
      // Currently only support for single Redis instance
      xRedis = \
        redis_factory->get_redis_interface(RedisConnectionList[0].ip, \
        RedisConnectionList[0].port);
    }
    catch (std::exception& e) {
      main_logging->error("Exception during Redis Initialization");
      main_logging->error(e.what());
      shutdown();
      exit(1);
    }
    main_logging->info("Connected to Redis");
  } else {
    main_logging->error("No Redis Connections found in configuration");
  }

  // Set up the Neo4j Connection
  std::string DBConnStr = cm->get_dbconnstr();
  try {
    neo = neo4j_factory->get_neo4j_interface(DBConnStr);
    main_logging->debug("Connected to Neo4j");
  }
  catch (std::exception& e) {
    main_logging->error("Exception during Neo4j Initialization");
    main_logging->error(e.what());
    shutdown();
    exit(1);
  }

  // Connect to the inbound ZMQ Admin
  std::string ib_zmq_connstr = cm->get_ibconnstr();
  if (!(ib_zmq_connstr.empty())) {
    zmqi = zmq_factory->get_zmq_inbound_interface(ib_zmq_connstr, REQ_RESP);
    main_logging->info("ZMQ Socket Open, opening request loop");
  } else {
    main_logging->error("No IB ZMQ Connection String Supplied");
    shutdown();
    exit(1);
  }

  // Set up the Message Processor
  processor = \
    processor_factory->build_processor(neo4j_factory, neo, xRedis, cm, ua);

  // Start and detach a thread to monitor the Kafka Queue
  std::thread km_thread(monitor_kafka_queue, cm->get_kafkabroker());
  km_thread.detach();

  // Main Request Loop

  while (true) {
    current_error_code = 100;
    current_error_message = "";
    msg_type = -1;
    std::string resp_str = "";
    rapidjson::Document d;
    std::string clean_string;

    // Convert the OMQ message into a string to be passed on the event
    char * req_ptr = zmqi->crecv();
    if (!req_ptr) {
      main_logging->error("Null Value returned from ZMQ");
      zmqi->send("{\"err_code\":120}");
      continue;
    }
    main_logging->debug("Conversion to C String performed with result: ");
    main_logging->debug(req_ptr);

    // Trim the string recieved
    std::string recvd_msg(req_ptr);

    // Protocol Buffer Format Type
    if (cm->get_formattype() == PROTO_FORMAT) {
      clean_string = trim(recvd_msg);
      main_logging->debug("Input String Cleaned");
      main_logging->debug(clean_string);

      try {
        new_proto.Clear();
        new_proto.ParseFromString(req_ptr);
        translated_object = scene_list_factory->build_scene(new_proto);
      }
      // Catch a possible error and write to logs
      catch (std::exception& e) {
        main_logging->error("Exception while parsing inbound document:");
        main_logging->error(e.what());
        current_error_code = TRANSLATION_ERROR;
        current_error_message = e.what();
      }

    } else if (cm->get_formattype() == JSON_FORMAT) {
      // Cleaning methods that only work on JSON
      int final_closing_char = recvd_msg.find_last_of("}");
      int first_opening_char = recvd_msg.find_first_of("{");
      clean_string = \
        recvd_msg.substr(first_opening_char, final_closing_char+1);
      main_logging->debug("Input String Cleaned");
      main_logging->debug(clean_string);

      try {
        d.Parse<rapidjson::kParseStopWhenDoneFlag>(clean_string.c_str());
        if (d.HasParseError()) {
          main_logging->error("Parsing Error: ");
          main_logging->error(GetParseError_En(d.GetParseError()));
          current_error_code = TRANSLATION_ERROR;
          current_error_message.assign(GetParseError_En(d.GetParseError()));
        } else {
          // Build the translated object from the document
          translated_object = scene_list_factory->build_scene(d);
        }
      }
      // Catch a possible error and write to logs
      catch (std::exception& e) {
        main_logging->error("Exception while parsing inbound document:");
        main_logging->error(e.what());
        current_error_code = TRANSLATION_ERROR;
        current_error_message = e.what();
      }
    }

    if ((current_error_code == TRANSLATION_ERROR) || (!translated_object)) {
      if (cm->get_formattype() == PROTO_FORMAT) {
        resp = scene_list_factory->build_protobuf_scene();
      } else {
        resp = scene_list_factory->build_json_scene();
      }
      resp->set_err_msg(current_error_message);
      resp->set_err_code(current_error_code);
      resp->set_msg_type(msg_type);
      std::string response_str;
      resp->to_msg_string(response_str);
      zmqi->send(response_str);
      delete resp;
      resp = NULL;
    } else {
      msg_type = translated_object->get_msg_type();
      translated_object->print();

      // Determine the Transaction ID
      UuidContainer id_container;
      id_container.id = "";
      if (cm->get_transactionidsactive()) {
        std::string existing_trans_id = \
          translated_object->get_transaction_id();
        // If no transaction ID is sent in, generate a new one
        if (existing_trans_id.empty()) {
          try {
            id_container = ua->generate();
            if (!id_container.err.empty()) {
              uuid_logging->error(id_container.err);
            }
            main_logging->debug("Generated Transaction ID: " + \
              id_container.id);

            // Assign Transaction ID
            if (!translated_object) {
              main_logging->debug("No translated object to assign ID to");
            } else {
              translated_object->set_transaction_id(id_container.id);
            }
          }
          catch (std::exception& e) {
            main_logging->error("Exception during UUID Generation");
            shutdown();
            exit(1);
          }
        } else {
          // Otherwise, use the existing transaction ID
          id_container.id = existing_trans_id;
        }
      }
      main_logging->debug("Transaction ID: ");
      main_logging->debug(id_container.id);

      // Process the translated object
      ProcessResult *response = \
        processor->process_message(translated_object);

      // Turn the response from the processor into a response for the client
      if (cm->get_formattype() == PROTO_FORMAT) {
        resp = scene_list_factory->build_protobuf_scene();
      } else {
        resp = scene_list_factory->build_json_scene();
      }
      SceneInterface *resp_data = scene_factory->build_scene();
      resp->set_err_msg(current_error_message);
      resp->set_err_code(current_error_code);
      resp->set_msg_type(msg_type);

      // If we have a create request, we get a key back from the processor
      if (msg_type == SCENE_CRT) {
        resp_data->set_key(response->get_return_string());
      } else if (translated_object->num_scenes() > 0) {
        // Otherwise, set the response key from the translated object
        resp_data->set_key(translated_object->get_scene(0)->get_key());
      } else {
        main_logging->error("Unable to stamp key on response message");
      }

      resp->add_scene(resp_data);

      // Send reply back to client
      std::string application_response;

      // Ping message, send back "success"
      if (msg_type == PING) {
        application_response = "{\"err_code\":100}";
      } else if (msg_type == KILL) {
        // Kill message, shut down
        application_response = "{\"err_code\":100}";
        zmqi->send(application_response);
        shutdown();
        exit(1);
      } else if (!(response->successful())) {
        // processing error result
        resp->set_err_code(response->get_error_code());
        resp->set_err_msg(response->get_error_description());
        // Send the Inbound response
        resp->to_msg_string(application_response);
      } else if (msg_type == SCENE_GET || msg_type == SCENE_ENTER || \
        msg_type == SCENE_LEAVE || msg_type == DEVICE_ALIGN) {
          // If we have a load request or a
          // registration/deregistration/alignment, we will have a
          // proto buffer/JSON string in the response from the processor
          application_response = response->get_return_string();
      } else {
        // We have a standard message
        // Send the Inbound response
        resp->to_msg_string(application_response);
      }

      main_logging->info("Sending Response");
      main_logging->info(application_response);
      zmqi->send(application_response);

      // Clear the response
      if (!resp) {
        main_logging->debug("Response Object not found for deletion");
      } else {
        delete resp;
        resp = NULL;
      }

      // Clear the translated object
      if (!translated_object) {
        main_logging->debug("Translated Object not found for deletion");
      } else {
        delete translated_object;
        translated_object = NULL;
      }

      delete response;
    }
  }
  return 0;
}
