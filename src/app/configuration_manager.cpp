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

#include "include/configuration_manager.h"

ConfigurationManager::~ConfigurationManager() {
  if (isConsulActive) {
    ca->deregister_service(*s);
    delete s;
    delete ca;
  }
  delete consul_factory;
  delete props_factory;
}

// ---------------------Internal Configuration Methods----------------------- //

// ---------------------------Configure from File---------------------------- //

// Configure based on a properties file
bool ConfigurationManager::configure_from_file(std::string file_path) {
  // Open the file
  config_logging->info("Opening properties file:");
  config_logging->info(file_path);

  // Get a properties file manager, which will let us acces the file as a map
  PropertiesReaderInterface *props = \
    props_factory->get_properties_reader_interface(file_path);

  if (props->opt_exist("DB_ConnectionString")) {
    DB_ConnStr = props->get_opt("DB_ConnectionString");
    config_logging->info("DB Connection String:");
    config_logging->info(DB_ConnStr);
  }
  if (props->opt_exist("0MQ_InboundConnectionString")) {
    OMQ_IBConnStr = props->get_opt("0MQ_InboundConnectionString");
    config_logging->info("Inbound 0MQ Connection:");
    config_logging->info(OMQ_IBConnStr);
  }
  if (props->opt_exist("0MQ_Hostname")) {
    hostname = props->get_opt("0MQ_Hostname");
    config_logging->info("Inbound 0MQ Hostname:");
    config_logging->info(hostname);
  }
  if (props->opt_exist("KafkaBrokerAddress")) {
    kafka_addr = props->get_opt("KafkaBrokerAddress");
    config_logging->info("Kafka Address:");
    config_logging->info(kafka_addr);
  }
  if (props->opt_exist("0MQ_Port")) {
    port = props->get_opt("0MQ_Port");
    config_logging->info("Inbound 0MQ Port:");
    config_logging->info(port);
  }
  if (props->opt_exist("Data_Format_Type")) {
    std::string param_value = props->get_opt("Data_Format_Type");
    if (param_value == "1" || param_value == "JSON" || param_value == "json" \
      || param_value == "Json") {
      format_type = JSON_FORMAT;
    } else if (param_value == "0" || param_value == "Protobuf" \
      || param_value == "protobuf") {
      format_type = PROTO_FORMAT;
    }
    config_logging->info("Inbound 0MQ Connection:");
    config_logging->info(OMQ_IBConnStr);
  }
  if (props->opt_exist("StampTransactionId")) {
    if (props->get_opt("StampTransactionId") == "True") {
      StampTransactionId = true;
      config_logging->info("Transaction ID's Enabled");
    } else {
      StampTransactionId = false;
      config_logging->info("Transaction ID's Disabled");
    }
  }

  delete props;
  return true;
}

// --------------------------Configure from Consul--------------------------- //

std::vector<std::string> ConfigurationManager::split(std::string inp_string, \
  char delim) {
  std::vector<std::string> elems;
  std::stringstream ss(inp_string);
  std::string item;
  while (std::getline(ss, item, delim)) {
    elems.push_back(item);
  }
  return elems;
}

std::string ConfigurationManager::get_consul_config_value(std::string key) {
  config_resp_str = "__NULLSTR__";
  // Get a JSON List of the responses
  std::string config_json = "";
  std::string query_key = "ivan/" + key;
  try {
    config_json = ca->get_config_value(query_key);
    config_logging->debug("Configuration JSON Retrieved:");
    config_logging->debug(config_json);
  }
  catch (std::exception& e) {
    config_logging->error("Exception during Consul Value Retrieval");
    config_logging->error(e.what());
    throw e;
  }

  // Parse the JSON Response
  rapidjson::Document d;

  if (!config_json.empty()) {
    try {
      config_logging->debug("Config Value retrieved from Consul:");
      config_logging->debug(key);
      config_logging->debug(config_json);
      const char * config_cstr = config_json.c_str();
      d.Parse(config_cstr);
    }
    // Catch a possible error and write to logs
    catch (std::exception& e) {
      config_logging->error("Exception while parsing Consul Service Response:");
      config_logging->error(e.what());
    }
  } else {
    config_logging->error("Configuration Value not found");
    config_logging->error(key);
    return config_resp_str;
  }

  // Get the object out of the array
  const rapidjson::Value& v = d[0];
  const rapidjson::Value& resp = v["Value"];
  if (resp.IsString()) {
    config_resp_str = resp.GetString();
    // Transform the object from base64
    return ca->base64_decode(config_resp_str);
  }
  return config_resp_str;
}

// Configure based on the Services List and Key/Value store from Consul
bool ConfigurationManager::configure_from_consul(std::string consul_path, \
  std::string ip, std::string port, std::string advertised_host) {
  ca = consul_factory->get_consul_interface(consul_path);
  config_logging->info("Connecting to Consul");
  config_logging->info(consul_path);

  // Now, use the Consul Admin to configure the app

  std::string internal_address;

  // Step 1a: Generate new connectivity information for the inbound
  // service from command line arguments
  if (ip == "localhost") {
    internal_address = "*";
  } else {
    internal_address = ip;
  }

  OMQ_IBConnStr = "tcp://" + internal_address + ":" + port;

  // Step 1b: Register the Service with Consul

  // Build a new service definition for this currently running instance of ivan
  std::string name = "Ivan";
  std::string advertised_addr;
  if (!(advertised_host.empty())) {advertised_addr.assign(advertised_host);}
  else {advertised_addr.assign(internal_address);}
  s = consul_factory->get_service_interface(node_id, name, \
    advertised_addr, port);
  s->add_tag("ZMQ");

  // Register the service
  bool register_success = false;
  try {
    register_success = ca->register_service(*s);
  }
  catch (std::exception& e) {
    config_logging->error("Exception encountered during Service Registration");
    config_logging->error(e.what());
    return false;
  }

  if (!register_success) {
    config_logging->error("Failed to register with Consul");
    return false;
  }

  // Step 2: Get the key-value information for deployment-wide config
  // Including OB ZeroMQ Connectivity
  DB_ConnStr = get_consul_config_value("DB_ConnectionString");
  config_logging->debug("Database Connection String:");
  config_logging->debug(DB_ConnStr);
  if (DB_ConnStr == "__NULLSTR__") return false;

  kafka_addr = get_consul_config_value("KafkaBrokerAddress");
  config_logging->debug("Kafka Address:");
  config_logging->debug(kafka_addr);
  if (kafka_addr == "__NULLSTR__") return false;

  std::string tran_ids_active = get_consul_config_value("StampTransactionId");
  config_logging->debug("Transaction IDs Enabled:");
  config_logging->debug(tran_ids_active);
  if (tran_ids_active == "__NULLSTR__") return false;
  if (tran_ids_active == "True") {
    StampTransactionId = true;
  } else {
    StampTransactionId = false;
  }

  std::string format_type_str = get_consul_config_value("Data_Format_Type");
  config_logging->debug("Data Format:");
  config_logging->debug(format_type_str);
  if (format_type_str == "__NULLSTR__") return false;
  if (format_type_str == "JSON" || format_type_str == "Json" \
    || format_type_str == "json" || format_type_str == "1") {
    format_type = JSON_FORMAT;
  } else if (format_type_str == "ProtoBuf" || format_type_str == "Protobuf" \
    || format_type_str == "protobuf" || format_type_str == "PROTOBUF" \
    || format_type_str == "0") {
    format_type = PROTO_FORMAT;
  }

  return true;
}

// ---------------------External Configuration Methods----------------------- //

// The publicly exposed configure function
bool ConfigurationManager::configure() {
  // Null Check
  if (!cli) {
    config_logging->error("Configure with null Command Line Interpreter");
    return false;
  } else {
    bool ret_val = false;
    bool configured = false;

    // See if we have any environment variables specified
    const char * env_consul_addr = std::getenv("CRAZYIVAN_CONSUL_ADDR");
    const char * env_ip = std::getenv("CRAZYIVAN_IP");
    const char * env_port = std::getenv("CRAZYIVAN_PORT");
    const char * env_db_addr = std::getenv("CRAZYIVAN_DB_ADDR");
    const char * env_conf_file = std::getenv("CRAZYIVAN_CONFIG_FILE");

    // Check if we have a configuration file specified
    if (env_conf_file) {
      std::string env_conf_loc(env_conf_file);
      ret_val = configure_from_file(env_conf_loc);
      configured = true;
    } else if (cli->opt_exist("-config-file")) {
      ret_val =  configure_from_file(cli->get_opt("-config-file"));
      configured = true;
    }

    // String variables to hold the hostname, port, and consul connection info
    std::string env_ip_str = "";
    std::string env_port_str = "";
    std::string env_consul_addr_str = "";
    std::string advertised_host = "";

    // Pull any command line and env variables for ip, port, and consul address
    if (env_consul_addr) env_consul_addr_str.assign(env_consul_addr);
    if (env_ip) env_ip_str.assign(env_ip);
    if (env_port) env_port_str.assign(env_port);
    if (cli->opt_exist("-ip")) env_ip_str.assign(cli->get_opt("-ip"));
    if (cli->opt_exist("-port")) env_port_str.assign(cli->get_opt("-port"));
    if (cli->opt_exist("-consul-addr")) {
      env_consul_addr_str.assign(cli->get_opt("-consul-addr"));
    }
    if (cli->opt_exist("-advertised-host")) {
      advertised_host.assign(cli->get_opt("-advertised-host"));
    }

    // If we had a hostname and port specified in the configuration file,
    // then we override to that
    if (!(hostname.empty())) env_ip_str.assign(hostname);
    if (!(port.empty())) env_port_str.assign(port);

    // Execute Consul Configuration
    if (!(env_consul_addr_str.empty() || env_ip_str.empty() \
      || env_port_str.empty()))  {
      ret_val = \
        configure_from_consul(env_consul_addr_str, env_ip_str, env_port_str, advertised_host);
      if (ret_val) {
        isConsulActive = true;
        configured = true;
      } else {
        config_logging->error("Configuration from Consul failed");
      }
    } else {
      config_logging->error("Not enough info provided to register with Consul");
    }

    // If we have nothing specified, look for an app.properties file
    if (!configured) {
      ret_val = configure_from_file("app.properties");
    }

    // Override DB options with command line/env variables
    if (cli->opt_exist("-db-addr")) {
      DB_ConnStr = cli->get_opt("-db-addr");
    } else if (env_db_addr) {
      std::string env_db_addr_str(env_db_addr);
      DB_ConnStr = env_db_addr_str;
    }

    return ret_val;
  }
  return false;
}
