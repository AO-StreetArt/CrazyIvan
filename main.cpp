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
#include "src/uuid.h"
#include "src/scene.h"
#include "src/Scene.pb.h"
#include "src/message_processor.h"

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#include "aossl/factory_cli.h"
#include "aossl/factory_neo4j.h"
#include "aossl/factory_logging.h"
#include "aossl/factory_redis.h"
#include "aossl/factory_uuid.h"
#include "aossl/factory_zmq.h"

#include "aossl/factory/neo4j_interface.h"
#include "aossl/factory/redis_interface.h"
#include "aossl/factory/logging_interface.h"
#include "aossl/factory/uuid_interface.h"
#include "aossl/factory/commandline_interface.h"

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
      if ( cli->opt_exist("-log-conf") ) {
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

      std::string service_instance_id = "CLyman-";
      try {
        service_instance_id = service_instance_id + generate_uuid();
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

      //Set up internal variables
      int current_error_code;
      std::string current_error_message;
      int msg_type = -1;
      rapidjson::Document d;
      protoScene::Scene new_proto;

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
        main_logging->debug("Connected to Mongo");
      }
      catch (std::exception& e) {
        main_logging->error("Exception encountered during Mongo Initialization");
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
      processor = new MessageProcessor (neo, xRedis, cm);

      //Main Request Loop

      while (true) {

        current_error_code = 100;
        current_error_message = "";
        msg_type = -1;
        std::string resp_str = "";


        //Convert the OMQ message into a string to be passed on the event
        std::string req_string = zmqi->recv();
        req_string = ltrim(req_string);
        const char * req_ptr = req_string.c_str();
        main_logging->debug("Conversion to C String performed with result: ");
        main_logging->debug(req_ptr);

        try {
          new_proto.Clear();
          new_proto.ParseFromString(req_string);
          translated_object = new Scene (new_proto);
          msg_type = new_proto.message_type();
        }
        //Catch a possible error and write to logs
        catch (std::exception& e) {
          main_logging->error("Exception occurred while parsing inbound document:");
          main_logging->error(e.what());
          current_error_code = TRANSLATION_ERROR;
          current_error_message = e.what();
        }

        //Determine the Transaction ID
        std::string tran_id_str = "";
        if ( cm->get_transactionidsactive() ) {
          std::string existing_trans_id = translated_object->get_transaction_id();
          //If no transaction ID is sent in, generate a new one
          if ( existing_trans_id.empty() ) {
            try {
              tran_id_str = generate_uuid();
              main_logging->debug("Generated Transaction ID: " + tran_id_str);

              if (!translated_object)
              {
                main_logging->debug("No translated object to assign Transaction ID to");
              }
              else {
                translated_object->set_transaction_id(tran_id_str);
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
            tran_id_str = existing_trans_id;
          }
        }
        main_logging->debug("Transaction ID: ");
        main_logging->debug(tran_id_str);

        //Process the translated object
        std::string process_result = processor->process_message(translated_object);

        // Turn the response from the processor into a response for the client
        resp = new Scene();
        resp->set_key(translated_object->get_key());
        resp->set_err_msg(current_error_message);
        resp->set_err_code(current_error_code);

        //  Send reply back to client
        //Ping message, send back "success"
        if (msg_type == PING) {
          zmqi->send( "success" );
        }

        //Kill message, shut down
        else if (msg_type == KILL) {
          zmqi->send( "success" );
          shutdown();
          exit(1);
        }

        //We have a get message, so we have a serialized object in the processor response
        //"-1", we have a processing error result
        else if (process_result == "-1") {
          resp->set_msg_type(PROCESSING_ERROR);
          resp->set_err_msg("Error encountered in document processing");
          //Send the Inbound response
          zmqi->send( resp->to_protobuf() );
          main_logging->debug("Response Sent");
        }

        //If we have a load request, we will have a proto buffer string
        //in the response from the processor
        else if (msg_type == SCENE_GET) {
          zmqi->send( process_result );
        }

        //We have a standard message
        else {
          //If we have a create request, we will get a key back from the processor
          if (msg_type == SCENE_CRT) {
            resp->set_key( process_result );
          }
          //Otherwise, set the response key from the translated object
          else {
            resp->set_key( translated_object->get_key() );
          }

          //Send the Inbound response
          zmqi->send( resp->to_protobuf() );
          main_logging->debug("Response Sent");
        }

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
      }
      return 0;
    }
