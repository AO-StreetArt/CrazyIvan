//This sets up all of the components necessary for the service and runs the main
//loop for the application.

#include <sstream>
#include <string>
#include <string.h>
#include <cstring>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <stdlib.h>
#include <unistd.h>
#include <exception>
#include <signal.h>

#include "src/ivan_log.h"
#include "src/ivan_utils.h"
#include "src/configuration_manager.h"
#include "src/globals.h"
#include "src/scene.h"
#include "src/Scene.pb.h"
#include "src/message_processor.h"

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/error/en.h"

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

enum {
  CACHE_TYPE_1,
  CACHE_TYPE_2,
  CACHE_TYPE_MAX,
};

//Catch a Signal (for example, keyboard interrupt)
void my_signal_handler(int s){
   main_logging->error("Caught signal");
   std::string signal_type = std::to_string(s);
   main_logging->error(signal_type);
   shutdown();
   exit(1);
}

    //-----------------------
    //------Main Method------
    //-----------------------

    int main( int argc, char** argv )
    {

      //Set up internal variables
      int current_error_code;
      std::string current_error_message;
      int msg_type = -1;
      protoScene::SceneList new_proto;

      //Set up a handler for any signal events so that we always shutdown gracefully
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

      //Set up our command line interpreter
      cli = cli_factory->get_command_line_interface( argc, argv );

      //Set up logging
	    std::string initFileName;

      //See if we have a command line setting for the log file
      const char * env_logging_file = std::getenv("CRAZYIVAN_LOGGING_CONF");
      if ( env_logging_file ) {
        std::string tempFileName (env_logging_file);
        initFileName = tempFileName;
      }
      else if ( cli->opt_exist("-log-conf") ) {
        initFileName = cli->get_opt("-log-conf");
      }
      else
      {
        initFileName = "log4cpp.properties";
      }

      //This reads the logging configuration file
      logging = logging_factory->get_logging_interface(initFileName);

      //Set up the logging submodules for each category
      start_logging_submodules();

      //Set up the UUID Generator
      ua = uuid_factory->get_uuid_interface();

      std::string service_instance_id = "Ivan-";
      UuidContainer sid_container;
      try {
        sid_container = ua->generate();
        if (!sid_container.err.empty()) {
          uuid_logging->error(sid_container.err);
        }
        service_instance_id = service_instance_id + sid_container.id;
      }
      catch (std::exception& e) {
        main_logging->error("Exception encountered during Service Instance ID Generation");
        shutdown();
        exit(1);
      }

      //Set up our configuration manager with the CLI
      cm = new ConfigurationManager(cli, service_instance_id);

      //The configuration manager will  look at any command line arguments,
      //configuration files, and Consul connections to try and determine the correct
      //configuration for the service

      bool config_success = false;
      try {
        config_success = cm->configure();
      }
      catch (std::exception& e) {
        main_logging->error("Exception encountered during Configuration");
        shutdown();
        exit(1);
      }
      if (!config_success)
      {
        main_logging->error("Configuration Failed, defaults kept");
      }

      //Set up our Redis Connection List, which is passed to the Redis Admin to connect
      std::vector<RedisConnChain> RedisConnectionList = cm->get_redisconnlist();
      //Set up Redis Connection
      if (RedisConnectionList.size() > 0) {
        try {
          //Currently only support for single Redis instance
          xRedis = redis_factory->get_redis_interface(RedisConnectionList[0].ip, RedisConnectionList[0].port);
        }
        catch (std::exception& e) {
          main_logging->error("Exception encountered during Redis Initialization");
          main_logging->error(e.what());
          shutdown();
          exit(1);
        }
        main_logging->info("Connected to Redis");
      }
      else {
        main_logging->error("No Redis Connections found in configuration");
      }

      //Set up the Neo4j Connection
      std::string DBConnStr = cm->get_dbconnstr();
      try {
        neo = neo4j_factory->get_neo4j_interface( DBConnStr );
        main_logging->debug("Connected to Neo4j");
      }
      catch (std::exception& e) {
        main_logging->error("Exception encountered during Neo4j Initialization");
        main_logging->error(e.what());
        shutdown();
        exit(1);
      }

      //Connect to the inbound ZMQ Admin
      std::string ib_zmq_connstr = cm->get_ibconnstr();
      if ( !(ib_zmq_connstr.empty()) ) {
        zmqi = zmq_factory->get_zmq_inbound_interface(ib_zmq_connstr, REQ_RESP);
        main_logging->info("ZMQ Socket Open, opening request loop");
      }
      else {
        main_logging->error("No IB ZMQ Connection String Supplied");
        shutdown();
        exit(1);
      }

      //Set up the Message Processor
      processor = new MessageProcessor (neo4j_factory, neo, xRedis, cm, ua);

      //Main Request Loop

      while (true) {

        current_error_code = 100;
        current_error_message = "";
        msg_type = -1;
        std::string resp_str = "";
        rapidjson::Document d;

        //Convert the OMQ message into a string to be passed on the event
        //std::string req_string = zmqi->recv();
        //req_string = ltrim(req_string);
        char * req_ptr = zmqi->crecv();
        main_logging->debug("Conversion to C String performed with result: ");
        main_logging->debug(req_ptr);

        //Trim the string recieved
        std::string recvd_msg (req_ptr);
        recvd_msg = trim(recvd_msg);
        std::string clean_string = recvd_msg.substr(0, recvd_msg.find_last_of("}")+1);

        main_logging->debug("Input String Cleaned");
        main_logging->debug(clean_string);

        //Protocol Buffer Format Type
        if (cm->get_formattype() == PROTO_FORMAT) {

          try {
            new_proto.Clear();
            new_proto.ParseFromString(req_ptr);
            translated_object = new Scene (new_proto);
            msg_type = new_proto.message_type();
            main_logging->debug("Translated Scene List:");
            translated_object->print();
          }
          //Catch a possible error and write to logs
          catch (std::exception& e) {
            main_logging->error("Exception occurred while parsing inbound document:");
            main_logging->error(e.what());
            current_error_code = TRANSLATION_ERROR;
            current_error_message = e.what();
          }

        }
        //JSON Format Type
        else if (cm->get_formattype() == JSON_FORMAT) {

          try {
            d.Parse<rapidjson::kParseStopWhenDoneFlag>(clean_string.c_str());
            if (d.HasParseError()) {
              main_logging->error("Parsing Error: ");
              main_logging->error(GetParseError_En(d.GetParseError()));
              current_error_code = TRANSLATION_ERROR;
              current_error_message.assign(GetParseError_En(d.GetParseError()));
            }
          }
          //Catch a possible error and write to logs
          catch (std::exception& e) {
            main_logging->error("Exception occurred while parsing inbound document:");
            main_logging->error(e.what());
            current_error_code = TRANSLATION_ERROR;
            current_error_message = e.what();
          }

        }

        if (current_error_code == TRANSLATION_ERROR) {
          resp = new Scene();
          resp->set_err_msg(current_error_message);
          resp->set_err_code(current_error_code);
          resp->set_msg_type(msg_type);
          if (cm->get_formattype() == PROTO_FORMAT) {
            zmqi->send( resp->to_protobuf() );
          }
          else if (cm->get_formattype() == JSON_FORMAT) {
            zmqi->send( resp->to_json() );
          }
          delete resp;
        } else {

          //Build the translated object from the document
          translated_object = new Scene (d);
          msg_type = translated_object->get_msg_type();
          translated_object->print();

          //Determine the Transaction ID
          UuidContainer id_container;
          id_container.id = "";
          if ( cm->get_transactionidsactive() ) {
            std::string existing_trans_id = translated_object->get_transaction_id();
            //If no transaction ID is sent in, generate a new one
            if ( existing_trans_id.empty() ) {
              try {
                id_container = ua->generate();
                if (!id_container.err.empty()) {
                  uuid_logging->error(id_container.err);
                }
                main_logging->debug("Generated Transaction ID: " + id_container.id);

                //Assign Transaction ID
                if (!translated_object)
                {
                  main_logging->debug("No translated object to assign Transaction ID to");
                }
                else {
                  translated_object->set_transaction_id(id_container.id);
                }
              }
              catch (std::exception& e) {
                main_logging->error("Exception encountered during UUID Generation");
                shutdown();
                exit(1);
              }
            }
            //Otherwise, use the existing transaction ID
            else {
              id_container.id = existing_trans_id;
            }
          }
          main_logging->debug("Transaction ID: ");
          main_logging->debug(id_container.id);

          //Process the translated object
          ProcessResult *response = processor->process_message(translated_object);

          // Turn the response from the processor into a response for the client
          resp = new Scene();
          SceneData *resp_data = new SceneData;
          resp->set_err_msg(current_error_message);
          resp->set_err_code(current_error_code);
          resp->set_msg_type(msg_type);

          //If we have a create request, we will get a key back from the processor
          if (msg_type == SCENE_CRT) {
            resp_data->set_key( response->get_return_string() );
          }
          //Otherwise, set the response key from the translated object
          else if (translated_object->num_scenes() > 0) {
            resp_data->set_key(translated_object->get_scene(0)->get_key());
          }
          else {
            main_logging->error("Unable to stamp key on response message");
          }

          resp->add_scene(resp_data);

          //  Send reply back to client
          std::string application_response = "";

          //Ping message, send back "success"
          if (msg_type == PING) {
            application_response = "{\"err_code\":100}";
          }

          //Kill message, shut down
          else if (msg_type == KILL) {
            application_response = "{\"err_code\":100}";
            shutdown();
            exit(1);
          }

          //We have a get message, so we have a serialized object in the processor response
          //"-1", we have a processing error result
          else if ( !(response->successful()) ) {
            resp->set_err_code( response->get_error_code() );
            resp->set_err_msg( response->get_error_description() );
            //Send the Inbound response
            if (cm->get_formattype() == PROTO_FORMAT) {
              application_response = resp->to_protobuf();
            }
            else if (cm->get_formattype() == JSON_FORMAT) {
              application_response = resp->to_json();
            }
          }

          //If we have a load request or a registration/deregistration/alignment,
          //we will have a proto buffer string
          //in the response from the processor
          else if (msg_type == SCENE_GET || msg_type == SCENE_ENTER || \
            msg_type == SCENE_LEAVE || msg_type == DEVICE_ALIGN) {
              application_response = response->get_return_string();
          }

          //We have a standard message
          else {

            //Send the Inbound response
            if (cm->get_formattype() == PROTO_FORMAT) {
              application_response = resp->to_protobuf();
            }
            else if (cm->get_formattype() == JSON_FORMAT) {
              application_response = resp->to_json();
            }
          }

          main_logging->info("Sending Response");
          main_logging->info( application_response );
          zmqi->send( application_response );

          //Clear the response
          if (!resp) {
            main_logging->debug("Response Object not found for deletion");
          }
          else {
            delete resp;
            resp = NULL;
          }

          //Clear the translated object
          if (!translated_object) {
            main_logging->debug("Translated Object not found for deletion");
          }
          else {
            delete translated_object;
            translated_object = NULL;
          }

          delete response;

          //If translated object
        }
      }
      return 0;
    }
