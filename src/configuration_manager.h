//This implements the Configuration Manager

//This takes in a Command Line Interpreter, and based on the options provided,
//decides how the application needs to be configured.  It may configure either
//from a configuration file, or from a Consul agent

#include <string>
#include <fstream>
#include <cstdlib>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sstream>
#include <fstream>
#include <vector>
#include <errno.h>
#include <sys/stat.h>

#include "ivan_log.h"

#include "aossl/interpreter/cli/include/commandline_interface.h"
#include "aossl/consul/include/consul_interface.h"
#include "aossl/interpreter/properties_reader/include/properties_reader_interface.h"
#include "aossl/logging/include/logging_interface.h"
#include "aossl/redis/include/redis_interface.h"

#include "aossl/consul/include/factory_consul.h"
#include "aossl/interpreter/properties_reader/include/factory_props.h"

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#ifndef CONFIG_MANAGER
#define CONFIG_MANAGER

class ConfigurationManager
{

ConsulComponentFactory *consul_factory = NULL;
PropertyReaderFactory *props_factory = NULL;

//Internal Consul Administrator
ConsulInterface *ca = NULL;
bool isConsulActive;

//Command Line Interpreter holding config arguments
CommandLineInterface *cli = NULL;

//Consul Service Definition
ServiceInterface *s = NULL;

//Configuration Variables
std::string DB_ConnStr;
std::string OMQ_IBConnStr;
std::vector<RedisConnChain> RedisConnectionList;
bool StampTransactionId;
bool AtomicTransactions;

//The Current Node ID
std::string node_id;

//String Manipulations

//Split a string, based on python's split method
std::vector<std::string> split(std::string inp_string, char delim);

//Internal Configuration Methods

//File Config
//bool file_exists (std::string name);
bool configure_from_file (std::string file_path);

//Consul Config
std::string get_consul_config_value(std::string key);
bool configure_from_consul (std::string consul_path, std::string ip, std::string port);

public:
  //Constructor
  //Provides a set of default values that allow ex38 to run locally in a 'dev' mode
  ConfigurationManager(CommandLineInterface *c, std::string instance_id) {cli = c;\
    DB_ConnStr="neo4j://neo4j:neo4j@localhost:7687"; OMQ_IBConnStr="tcp://*:5555";\
      isConsulActive=false;StampTransactionId=false;AtomicTransactions=false;\
        node_id=instance_id;consul_factory=new ConsulComponentFactory;\
          props_factory = new PropertyReaderFactory;}
  ~ConfigurationManager();

  //Populate the configuration variables
  bool configure();

  //Get configuration values
  std::string get_dbconnstr() {return DB_ConnStr;}
  std::string get_ibconnstr() {return OMQ_IBConnStr;}
  std::vector<RedisConnChain> get_redisconnlist() {return RedisConnectionList;}
  bool get_transactionidsactive() {return StampTransactionId;}
  bool get_atomictransactions() {return AtomicTransactions;}

  //Get the Current Node ID
  std::string get_nodeid() {return node_id;}
};

#endif
