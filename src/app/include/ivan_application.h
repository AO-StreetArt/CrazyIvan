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

#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <cstdlib>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <exception>
#include <thread>

#include "proc/processor/include/processor_interface.h"
#include "proc/processor/include/processor_factory.h"

#include "cache/include/device_cache.h"
#include "cache/include/cache_loader.h"

#include "controller/include/scene_base_handler.h"
#include "controller/include/scene_update_handler.h"
#include "controller/include/scene_delete_handler.h"
#include "controller/include/scene_handler_factory.h"

#include "user/include/account_manager_interface.h"
#include "user/include/account_manager.h"

#include "cache_loader_process.h"
#include "event_stream_process.h"
#include "thread_error_handler.h"

#include "aossl/profile/include/tiered_app_profile.h"
#include "aossl/consul/include/consul_interface.h"
#include "aossl/consul/include/factory_consul.h"

#include "neocpp/connection/interface/neo4j_interface.h"
#include "neocpp/connection/impl/libneo4j_factory.h"

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/error/en.h"

#include "Poco/Net/HTTPServer.h"
#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPRequestHandlerFactory.h"
#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Net/SecureServerSocket.h"
#include "Poco/Net/SocketAddress.h"
#include "Poco/Net/ServerSocket.h"
#include "Poco/Util/ServerApplication.h"
#include "Poco/Util/AbstractConfiguration.h"
#include "Poco/Util/Option.h"
#include "Poco/Util/OptionSet.h"

#include "Poco/Logger.h"
#include "Poco/PatternFormatter.h"
#include "Poco/FormattingChannel.h"
#include "Poco/ConsoleChannel.h"
#include "Poco/FileChannel.h"
#include "Poco/Message.h"
#include "Poco/NumberParser.h"
#include "Poco/AutoPtr.h"
#include "Poco/AsyncChannel.h"
#include "Poco/ErrorHandler.h"

#ifndef SRC_APP_INCLUDE_IVAN_APPLICATION_H_
#define SRC_APP_INCLUDE_IVAN_APPLICATION_H_

// The main Ivan Application

// Extends a Poco ServerApplication and let's it handle
// the heavy lifting.  Internally it will spin up a separate
// thread to handle each request.
class CrazyIvan: public Poco::Util::ServerApplication {
  ProcessorInterface *proc = NULL;
  Neocpp::Neo4jInterface *neo = NULL;
  AccountManagerInterface *acct_manager = NULL;
public:
 CrazyIvan() {}
 ~CrazyIvan() {}
protected:

  // Initialize the core components of the application
  void initialize(Poco::Util::Application& self) {
    // Have Poco load the configuration file to provide SSL configuration
    loadConfiguration("ssl.properties");
    Poco::Util::ServerApplication::initialize(self);
  }

  // App Shutdown Routine
  void uninitialize() {
    Poco::Util::ServerApplication::uninitialize();
    if (proc) delete proc;
    if (neo) delete neo;
    if (acct_manager) delete acct_manager;
  }

  // Define basic CLI Opts
  void defineOptions(Poco::Util::OptionSet& options) {
    Poco::Util::ServerApplication::defineOptions(options);
    // options.addOption(Poco::Util::Option("cluster", "c", "Cluster Name")\
    //   .required(false).argument("value").binding("cluster.name"));
  }

  // Main application method
  int main(const std::vector<std::string>& args) {
    Poco::Util::Application& app = Poco::Util::Application::instance();
    app.logger().information("Starting Crazy Ivan");
    is_app_running = true;
    for (std::string elt : args) {
      app.logger().information(elt);
    }
    // Initialize the application profile
    AOSSL::TieredApplicationProfile config(args, std::string("CrazyIvan"), std::string("prod"));
    // Set default values for configuration
    config.add_opt(std::string("connection.neo4j"), \
      std::string("neo4j://localhost:7687"));
    config.add_opt(std::string("transaction.format"), std::string("json"));
    config.add_opt(std::string("transaction.id.stamp"), std::string("true"));
    config.add_opt(std::string("event.stream.method"), std::string("kafka"));
    config.add_opt(std::string("event.format"), std::string("json"));
    config.add_opt(std::string("http.host"), std::string("localhost"));
    config.add_opt(std::string("http.port"), std::string("8765"));
    config.add_opt(std::string("udp.port"), std::string("8764"));
    config.add_opt(std::string("cluster.name"), std::string("default"));
    config.add_opt(std::string("log.file"), std::string("ivan.log"));
    config.add_opt(std::string("log.level"), std::string("Info"));
    config.add_opt(std::string("transaction.security.ssl.enabled"), std::string("false"));
    config.add_opt(std::string("transaction.security.auth.type"), std::string("none"));
    config.add_opt(std::string("transaction.security.auth.user"), std::string("test"));
    config.add_opt(std::string("transaction.security.auth.password"), std::string("test"));
    config.add_opt(std::string("transaction.security.hash.password"), std::string("test"));
    config.add_opt(std::string("event.security.aes.enabled"), std::string("false"));
    config.add_opt(std::string("event.security.out.aes.key"), std::string("s3cr3tk3y"));
    config.add_opt(std::string("event.security.out.aes.salt"), std::string("asdff8723lasdf(**923412"));
    config.add_opt(std::string("event.security.in.aes.key"), std::string("4n0th3rk4y"));
    config.add_opt(std::string("event.security.in.aes.salt"), std::string("asdff8723lasdf(**923412"));
    // Perform the initial config
    bool config_success = false;
    bool config_tried = false;
    int config_attempts = 0;
    while (!config_success) {
      if (config_attempts > 50) {
        app.logger().error("Max Config Attempts failed, exiting");
        return Poco::Util::Application::EXIT_NOINPUT;
      }
      if (config_tried) {
        app.logger().error("Configuration Failed, trying again in 5 seconds");
        usleep(5000000);
      } else {
        config_tried = true;
      }
      try {
        config.load_config();
        config_success = true;
      }
      catch (std::exception& e) {
        app.logger().error("Exception encountered during Configuration");
        config_attempts++;
      }
    }

    // Set up Poco Logging framework
    // Set up a console channel, and an async channel so that
    // log lines are written on a background thread
    Poco::AutoPtr<Poco::ConsoleChannel> pCons(new Poco::ConsoleChannel);
    Poco::AutoPtr<Poco::AsyncChannel> pAsync(new Poco::AsyncChannel(pCons));
    Poco::FormattingChannel* console_channel = \
      new Poco::FormattingChannel(new Poco::PatternFormatter("%Y-%m-%d %H:%M:%S.%c %N[%P]:%s:%q:%t"));
    console_channel->setChannel(pAsync);
    console_channel->open();

    // Start up the app loggers
    AOSSL::StringBuffer log_level;
    config.get_opt(std::string("log.level"), log_level);
    int log_priority;
    if (log_level.val == "Debug") {
      log_priority = Poco::Message::PRIO_DEBUG;
    } else if (log_level.val == "Warn") {
      log_priority = Poco::Message::PRIO_WARNING;
    } else if (log_level.val == "Error") {
      log_priority = Poco::Message::PRIO_ERROR;
    } else {
      app.logger().error("Unable to read log level: %s, defaulting to info", log_level.val);
      log_priority = Poco::Message::PRIO_INFORMATION;
    }
    Poco::Logger& main_logger = Poco::Logger::create("MainLogger", \
      console_channel, log_priority);
    Poco::Logger& process_logger = Poco::Logger::create("MessageProcessor", \
      console_channel, log_priority);
    Poco::Logger& controller_logger = Poco::Logger::create("Controller", \
      console_channel, log_priority);
    Poco::Logger& data_logger = Poco::Logger::create("Data", \
      console_channel, log_priority);
    Poco::Logger& auth_logger = Poco::Logger::create("Auth", \
      console_channel, log_priority);
    Poco::Logger& cache_logger = Poco::Logger::create("Cache", \
      console_channel, log_priority);
    Poco::Logger& event_logger = Poco::Logger::create("Event", \
      console_channel, log_priority);
    data_logger.information("Data Logger Initialized");
    controller_logger.information("Controller Logger Initialized");
    process_logger.information("Processor Logger Initialized");
    auth_logger.information("Authorization Logger Initialized");
    cache_logger.information("Cache Logger Initialized");
    event_logger.information("Event Stream Logger Initialized");
    main_logger.information("Logging Configuration complete");

    // std::string cluster_name_lbl = "cluster.name";
    // std::string cluster_name_val = app.config().getString(cluster_name_lbl);
    // main_logger.information(cluster_name_val);

    // Register the service with Consul
    AOSSL::StringBuffer http_host;
    config.get_opt(std::string("http.host"), http_host);
    AOSSL::StringBuffer http_port;
    config.get_opt(std::string("http.port"), http_port);
    if (config.get_consul()) {
      main_logger.information("Registering with Consul");
      AOSSL::StringBuffer cluster_name;
      config.get_opt(std::string("cluster.name"), cluster_name);
      std::vector<std::string> tags;
      tags.push_back(std::string("cluster=") + cluster_name.val);
      AOSSL::ConsulComponentFactory consul_factory;
      AOSSL::ServiceInterface *my_app = \
        consul_factory.get_service_interface(std::string("CrazyIvan"), \
        std::string("CrazyIvan"), http_host.val, http_port.val, tags);
      config.get_consul()->register_service(*my_app);
    }

    // Set up the Neo4j Connection
    main_logger.information("Creating Neo4j Interface");
    AOSSL::StringBuffer neo_addr;
    config.get_opt(std::string("connection.neo4j"), neo_addr);
    main_logger.information("Neo4j Connection registered: %s", neo_addr.val);
    Neocpp::LibNeo4jFactory neo_factory;
    neo = neo_factory.get_neo4j_interface(neo_addr.val);

    // Start the Message Processor
    ProcessorFactory process_factory;
    proc = process_factory.build_processor(&neo_factory, \
      neo, &config, config.get_uuid());

    // Start the User Account Manager
    AOSSL::StringBuffer auth_type_buffer;
    AOSSL::StringBuffer auth_un_buffer;
    AOSSL::StringBuffer auth_pw_buffer;
    AOSSL::StringBuffer hash_pw_buffer;
    config.get_opt(std::string("transaction.security.auth.type"), auth_type_buffer);
    if (auth_type_buffer.val == "single") {
      config.get_opt(std::string("transaction.security.auth.user"), auth_un_buffer);
      config.get_opt(std::string("transaction.security.auth.password"), auth_pw_buffer);
      config.get_opt(std::string("transaction.security.hash.password"), hash_pw_buffer);
      acct_manager = new SingleAccountManager(auth_un_buffer.val, auth_pw_buffer.val, hash_pw_buffer.val);
    } else {
      acct_manager = NULL;
    }

    // Start the background thread error handler
    IvanErrorHandler eh;
    Poco::ErrorHandler* pOldEH = Poco::ErrorHandler::set(&eh);

    // Start the Device Cache
    DeviceCache event_cache;
    DeviceCacheLoader loader(&event_cache, neo);

    // Kick off the Cache Loader background thread
    std::thread cl_thread(load_device_cache, &loader, 5000000);
    cl_thread.detach();

    // Kick off the Event Stream background thread
    std::thread es_thread(event_stream, &event_cache, &config);
    es_thread.detach();

    // Main Application Loop (Serving HTTP API)
    AOSSL::StringBuffer ssl_enabled_buf;
    config.get_opt(std::string("transaction.security.ssl.enabled"), ssl_enabled_buf);
    std::string http_address = http_host.val + std::string(":") + http_port.val;
    Poco::Net::SocketAddress saddr(http_address);
    main_logger.debug("HTTP Address: %s, Security Enabled: %s", http_address, ssl_enabled_buf.val);
    if (ssl_enabled_buf.val == "true") {
      main_logger.information("Opening Secure HTTP Socket");
      Poco::Net::SecureServerSocket svs(saddr, 64);
      Poco::Net::HTTPServer srv(new SceneHandlerFactory(&config, proc, acct_manager, &event_cache), svs, \
        new Poco::Net::HTTPServerParams);
      srv.start();
      waitForTerminationRequest();
      srv.stop();
    } else {
      main_logger.information("Opening HTTP Socket");
      Poco::Net::ServerSocket svs(saddr);
      Poco::Net::HTTPServer srv(new SceneHandlerFactory(&config, proc, acct_manager, &event_cache), svs, \
        new Poco::Net::HTTPServerParams);
      srv.start();
      waitForTerminationRequest();
      srv.stop();
    }
    // Handle graceful shutdown of background threads
    main_logger.information("Shutting down application");
    is_app_running = false;
    while(is_sender_running.load()) {usleep(1000000);}
    Poco::ErrorHandler::set(pOldEH);
    return Poco::Util::Application::EXIT_OK;
  }
};

#endif  // SRC_APP_INCLUDE_IVAN_APPLICATION_H_
