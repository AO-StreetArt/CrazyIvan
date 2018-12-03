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

#include "controller/include/scene_handler_factory.h"

#include "user/include/account_manager_interface.h"
#include "user/include/account_manager.h"

#include "loader_process.h"
#include "event_stream_process.h"
#include "thread_error_handler.h"
#include "database_manager.h"

#include "aossl/core/include/buffers.h"
#include "aossl/core/include/base_http_client.h"
#include "aossl/profile/include/network_app_profile.h"
#include "aossl/consul/include/consul_interface.h"
#include "aossl/consul/include/factory_consul.h"
#include "aossl/vault/include/vault_interface.h"

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/error/en.h"

#include "Poco/Net/Context.h"
#include "Poco/Net/HTTPServer.h"
#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPRequestHandlerFactory.h"
#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Net/SecureServerSocket.h"
#include "Poco/Net/ServerSocket.h"
#include "Poco/Net/ConsoleCertificateHandler.h"
#include "Poco/Net/KeyConsoleHandler.h"
#include "Poco/Net/SocketAddress.h"
#include "Poco/Net/SSLManager.h"
#include "Poco/Util/AbstractConfiguration.h"
#include "Poco/Util/ServerApplication.h"
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
  bool need_to_cleanup_neo4j_interface = false;
  AccountManagerInterface *acct_manager = NULL;
  AOSSL::ServiceInterface *my_app = NULL;
  AOSSL::ServiceInterface *my_app_udp = NULL;
  AOSSL::BaseHttpClient *http_client = nullptr;
public:
 CrazyIvan() {}
 ~CrazyIvan() {}
protected:

  // Initialize the core components of the application
  void initialize(Poco::Util::Application& self) {
    // Have Poco load the configuration file to provide SSL configuration
    Poco::Util::ServerApplication::initialize(self);
  }

  // App Shutdown Routine
  void uninitialize() {
    Poco::Util::ServerApplication::uninitialize();
    if (proc) delete proc;
    if (neo && need_to_cleanup_neo4j_interface) delete neo;
    if (acct_manager) delete acct_manager;
    if (http_client) delete http_client;
  }

  // Define basic CLI Opts
  void defineOptions(Poco::Util::OptionSet& options) {
    Poco::Util::ServerApplication::defineOptions(options);
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
    AOSSL::NetworkApplicationProfile config(args, std::string("ivan"), std::string("prod"));

    // Print the configuration log
    app.logger().information("Profile Startup Log");
    std::vector<std::string> profile_startup_log = config.get_config_record();
    for (auto& log_line: profile_startup_log) {
      app.logger().information(log_line);
    }

    // Add secure opts
    config.add_secure_opt(config.get_cluster_name() + \
        std::string(".transaction.security.auth.user"));
    config.add_secure_opt(config.get_cluster_name() + \
        std::string(".transaction.security.auth.password"));
    config.add_secure_opt(config.get_cluster_name() + \
        std::string(".transaction.security.hash.password"));
    config.add_secure_opt(config.get_cluster_name() + \
        std::string(".event.security.out.aes.key"));
    config.add_secure_opt(config.get_cluster_name() + \
        std::string(".event.security.out.aes.iv"));
    config.add_secure_opt(config.get_cluster_name() + \
        std::string(".event.security.out.aes.password"));
    config.add_secure_opt(config.get_cluster_name() + \
        std::string(".event.security.out.aes.salt"));
    config.add_secure_opt(config.get_cluster_name() + \
        std::string(".event.security.registration.aes.key"));
    config.add_secure_opt(config.get_cluster_name() + \
        std::string(".event.security.registration.aes.salt"));
    config.add_secure_opt(config.get_cluster_name() + \
        std::string(".event.security.registration.aes.password"));
    config.add_secure_opt(config.get_cluster_name() + \
        std::string(".event.security.registration.aes.iv"));
    config.add_secure_opt(config.get_cluster_name() + \
        std::string(".event.security.in.aes.key"));
    config.add_secure_opt(config.get_cluster_name() + \
        std::string(".event.security.in.aes.salt"));
    config.add_secure_opt(config.get_cluster_name() + \
        std::string(".event.security.in.aes.iv"));
    config.add_secure_opt(config.get_cluster_name() + \
        std::string(".event.security.in.aes.password"));
    config.add_secure_opt(std::string("neo4j.auth.un"));
    config.add_secure_opt(std::string("neo4j.auth.pw"));

    // Set default values for configuration
    config.add_opt(std::string("adrestia"), \
      std::string("http://localhost:8080"));
    config.add_opt(std::string("adrestia.discover"), std::string("false"));
    config.add_opt(std::string("adrestia.secure"), std::string("false"));
    config.add_opt(std::string("adrestia.cache.load"), std::string("false"));
    config.add_opt(std::string("clyman.service.name"), std::string("Clyman"));
    config.add_opt(std::string("neo4j"), \
      std::string("neo4j://localhost:7687"));
    config.add_opt(std::string("neo4j.discover"), std::string("false"));
    config.add_opt(std::string("neo4j.ssl.ca.file"), std::string(""));
    config.add_opt(std::string("neo4j.ssl.ca.dir"), std::string(""));
    config.add_opt(std::string("transaction.format"), std::string("json"));
    config.add_opt(std::string("transaction.id.stamp"), std::string("true"));
    config.add_opt(std::string("event.stream.method"), std::string("kafka"));
    config.add_opt(std::string("event.format"), std::string("json"));
    config.add_opt(std::string("http.host"), std::string("localhost"));
    config.add_opt(std::string("http.port"), std::string("8765"));
    config.add_opt(std::string("udp.port"), std::string("8764"));
    config.add_opt(std::string("log.file"), std::string("ivan.log"));
    config.add_opt(std::string("log.level"), std::string("Info"));
    config.add_opt(std::string("transaction.security.ssl.ca.vault.active"), std::string("false"));
    config.add_opt(std::string("transaction.security.ssl.ca.vault.role_name"), std::string("test"));
    config.add_opt(std::string("transaction.security.ssl.ca.vault.common_name"), std::string("test"));
    config.add_opt(std::string("transaction.security.ssl.enabled"), std::string("false"));
    config.add_opt(std::string("transaction.security.auth.type"), std::string("none"));
    config.add_opt(std::string("event.security.aes.enabled"), std::string("false"));

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
    Poco::Logger& database_manager_logger = Poco::Logger::create("DatabaseManager", \
      console_channel, log_priority);
    data_logger.information("Data Logger Initialized");
    controller_logger.information("Controller Logger Initialized");
    process_logger.information("Processor Logger Initialized");
    auth_logger.information("Authorization Logger Initialized");
    cache_logger.information("Cache Logger Initialized");
    event_logger.information("Event Stream Logger Initialized");
    database_manager_logger.information("Database Manager Logger Initialized");
    main_logger.information("Logging Configuration complete");

    main_logger.information("Profile Configuration Log");
    std::vector<std::string> profile_config_log = config.get_config_record();
    for (auto& log_line: profile_config_log) {
      main_logger.information(log_line);
    }

    // Set up the Neo4j Connection
    Neocpp::LibNeo4jFactory neo_factory;
    DatabaseManager neo4j_manager(&config);

    AOSSL::StringBuffer neo4j_discover_buf;
    config.get_opt(std::string("neo4j.discover"), neo4j_discover_buf);
    if (neo4j_discover_buf.val == "true") {
      // Use the DatabaseManager as the Neo4j interface
      // This handles service discovery and failover
      // between databases
      main_logger.information("Registering DB Manager as Neo4j Interface");
      neo = &neo4j_manager;
    } else {
      // Get the connection string for a single instance
      main_logger.information("Creating Neo4j Interface");
      AOSSL::StringBuffer neo_addr;
      config.get_opt(std::string("neo4j"), neo_addr);
      main_logger.information("Neo4j Connection registered: %s", neo_addr.val);
      // Initialize Neo4j Connection
      neo = neo_factory.get_neo4j_interface(neo_addr.val);
      need_to_cleanup_neo4j_interface = true;
    }

    // Start the Message Processor
    ProcessorFactory process_factory;
    proc = process_factory.build_processor(&neo_factory, \
      neo, &config, config.get_uuid(), config.get_cluster_name());

    // Start the User Account Manager
    AOSSL::StringBuffer auth_type_buffer;
    AOSSL::StringBuffer auth_un_buffer;
    AOSSL::StringBuffer auth_pw_buffer;
    AOSSL::StringBuffer hash_pw_buffer;
    config.get_opt(std::string("transaction.security.auth.type"), auth_type_buffer);
    if (auth_type_buffer.val == "single") {
      config.get_opt(config.get_cluster_name() + \
          std::string(".transaction.security.auth.user"), auth_un_buffer);
      config.get_opt(config.get_cluster_name() + \
          std::string(".transaction.security.auth.password"), auth_pw_buffer);
      config.get_opt(config.get_cluster_name() + \
          std::string(".transaction.security.hash.password"), hash_pw_buffer);
      acct_manager = new SingleAccountManager(auth_un_buffer.val, auth_pw_buffer.val, hash_pw_buffer.val);
    } else {
      acct_manager = NULL;
    }

    // Start the Device Cache
    DeviceCache event_cache;
    DeviceCacheLoader loader(&event_cache, neo);

    // Query Adrestia to see if we have any scenes to load into the cache
    AOSSL::StringBuffer adrestia_buffer;
    AOSSL::StringBuffer adrestia_secure_buffer;
    AOSSL::StringBuffer adrestia_discover_buffer;
    AOSSL::StringBuffer adrestia_load_cache_buffer;
    config.get_opt("adrestia.cache.load", adrestia_load_cache_buffer);
    config.get_opt("adrestia.discover", adrestia_discover_buffer);
    config.get_opt("adrestia.secure", adrestia_secure_buffer);
    config.get_opt("adrestia", adrestia_buffer);
    if (adrestia_load_cache_buffer.val == "true") {
      std::string adrestia_address = "";
      if (adrestia_secure_buffer.val == "true") {
        adrestia_address = "https://";
      } else {
        adrestia_address = "http://";
      }
      if (adrestia_discover_buffer.val == "true") {
        // Discover an Adrestia Instance
        AOSSL::ServiceInterface *adrestia = config.get_service("Adrestia");
        if (adrestia) {
          adrestia_address = adrestia_address + adrestia->get_address() + ":" + adrestia->get_port();
        }
        delete adrestia;
      } else {
        // Use the provided Adrestia instance
        adrestia_address = adrestia_buffer.val;
      }

      // Execute the HTTP request
      if (adrestia_secure_buffer.val == "true") {
        std::string adrestia_cert = "";
        http_client = new AOSSL::BaseHttpClient(adrestia_address, 5, adrestia_cert);
      } else {
        http_client = new AOSSL::BaseHttpClient(adrestia_address, 5);
      }
      std::string adrestia_query_url = "/cluster/" + config.get_cluster_name();
      AOSSL::StringBuffer adrestia_response;
      http_client->get_by_reference(adrestia_query_url, adrestia_response);

      // Parse the response and add elements to the scene cache
      rapidjson::Document doc;
      doc.Parse<rapidjson::kParseStopWhenDoneFlag>(adrestia_response.val.c_str());
      if (doc.HasParseError()) {
        main_logger.error("Failed to parse Adrestia response");
        main_logger.error(rapidjson::GetParseError_En(doc.GetParseError()));
      } else {
        main_logger.information("Loading Scenes into Cache from Adrestia");
        if (doc.IsArray()) {
          for (auto& itr : doc.GetArray()) {
            if (itr.IsString()) {
              event_cache.add_scene(itr.GetString());
            }
          }
        }
      }
    }

    AOSSL::StringBuffer ssl_enabled_buf;
    config.get_opt(std::string("transaction.security.ssl.enabled"), ssl_enabled_buf);

    // Look to see if we should be generating our SSL Certs from Vault
    AOSSL::StringBuffer use_vault_ca_buf;
    AOSSL::StringBuffer vault_role_buf;
    AOSSL::StringBuffer vault_common_name_buf;
    config.get_opt(std::string("transaction.security.ssl.ca.vault.active"), use_vault_ca_buf);
    config.get_opt(std::string("transaction.security.ssl.ca.vault.role_name"), vault_role_buf);
    config.get_opt(std::string("transaction.security.ssl.ca.vault.common_name"), vault_common_name_buf);
    if (use_vault_ca_buf.val == "true") {
      // Generate a new SSL Cert from Vault
      AOSSL::SslCertificateBuffer ssl_cert_buf;
      try {
        config.get_vault()->gen_ssl_cert(vault_role_buf.val, vault_common_name_buf.val, ssl_cert_buf);
      } catch (std::exception& e) {
        main_logger.error("Exception Generating SSL Cert from Vault, exiting");
        return Poco::Util::Application::EXIT_CONFIG;
      }
      if (ssl_cert_buf.success) {
        // We need to write the certificate, private key, and CA to files
        std::ofstream out_key("private.key");
        out_key << ssl_cert_buf.private_key;
        out_key.close();
        std::ofstream out_cert("cert.pem");
        out_cert << ssl_cert_buf.certificate;
        out_cert.close();
        std::ofstream out_ca("rootcert.pem");
        out_ca << ssl_cert_buf.issuing_ca;
        out_ca.close();
        bool use_chain_certs = false;
        std::string chain_cert_path = "cachain.pem";
        if (ssl_cert_buf.ca_chain.empty()) {
          main_logger.debug("Empty CA Chain detected, ignoring");
        } else {
          use_chain_certs = true;
        }
        std::ofstream out_chain(chain_cert_path);
        out_chain << ssl_cert_buf.ca_chain;
        out_chain.close();
        // Initialize the SSL Manager with those files
        Poco::SharedPtr<Poco::Net::PrivateKeyPassphraseHandler> pConsoleHandler = new Poco::Net::KeyConsoleHandler(true);
        Poco::SharedPtr<Poco::Net::InvalidCertificateHandler> pInvalidCertHandler = new Poco::Net::ConsoleCertificateHandler(true);
        Poco::Net::Context::Ptr pContext = new Poco::Net::Context(Poco::Net::Context::SERVER_USE, "private.key", "cert.pem", "rootcert.pem", Poco::Net::Context::VERIFY_NONE, 9, false, "ALL:!ADH:!LOW:!EXP:!MD5:@STRENGTH");
        if (use_chain_certs) {
          Poco::Net::X509Certificate chain_cert(chain_cert_path);
          main_logger.debug("Adding Chain Certificate to SSL Context");
          pContext->addChainCertificate(chain_cert);
        }
        Poco::Net::SSLManager::instance().initializeServer(pConsoleHandler, pInvalidCertHandler, pContext);
      }
    } else {
      if (ssl_enabled_buf.val == "true") {
        bool ssl_properties_loaded = false;
        // If we aren't generating ssl certs from Consul and we still need
        // to start an https endpoint, then look for an ssl.properties file
        // In the application root folder
        if (file_exists("ssl.properties")) {
          loadConfiguration("ssl.properties");
          ssl_properties_loaded = true;
        }
        // In the /etc folder (when running as a Linux Service)
        if (file_exists("/etc/ivan/ssl.properties")) {
          loadConfiguration("/etc/ivan/ssl.properties");
          ssl_properties_loaded = true;
        }
        // In the /ssl folder (when running in a container)
        if (file_exists("/ssl/ivan/ssl.properties")) {
          loadConfiguration("/ssl/ivan/ssl.properties");
          ssl_properties_loaded = true;
        }
        if (!ssl_properties_loaded) {
          main_logger.error("Unable to find required SSL Configuration to load, exiting");
          return Poco::Util::Application::EXIT_CONFIG;
        }
      }
    }

    // Register the service with Consul
    AOSSL::StringBuffer http_host;
    config.get_opt(std::string("http.host"), http_host);
    AOSSL::StringBuffer http_port;
    config.get_opt(std::string("http.port"), http_port);
    AOSSL::StringBuffer udp_port;
    config.get_opt(std::string("udp.port"), udp_port);
    if (config.get_consul()) {
      main_logger.information("Registering with Consul");
      std::vector<std::string> tags;
      tags.push_back(std::string("cluster=") + config.get_cluster_name());
      AOSSL::ConsulComponentFactory consul_factory;
      // Create the actual Service to register
      my_app = \
        consul_factory.get_service_interface(std::string("CrazyIvan"), \
        std::string("CrazyIvan"), http_host.val, http_port.val, tags);
      config.get_consul()->register_service(*my_app);
      my_app_udp = \
        consul_factory.get_service_interface(std::string("CrazyIvan_Udp"), \
        std::string("CrazyIvan_Udp"), http_host.val, udp_port.val, tags);
      config.get_consul()->register_service(*my_app);
    }

    // Start the background thread error handler
    IvanErrorHandler eh;
    Poco::ErrorHandler* pOldEH = Poco::ErrorHandler::set(&eh);

    // Kick off the Loader background thread
    std::thread cl_thread(load_data, &config, &loader, 5000000);
    cl_thread.detach();

    // Kick off the Event Stream background thread
    std::thread es_thread(event_stream, &event_cache, &config);
    es_thread.detach();

    // Main Application Loop (Serving HTTP API)
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
    if (my_app) {
      config.get_consul()->deregister_service(*my_app);
      delete my_app;
    }
    if (my_app_udp) {
      config.get_consul()->deregister_service(*my_app_udp);
      delete my_app_udp;
    }
    while(is_sender_running.load()) {usleep(1000000);}
    Poco::ErrorHandler::set(pOldEH);
    return Poco::Util::Application::EXIT_OK;
  }
};

#endif  // SRC_APP_INCLUDE_IVAN_APPLICATION_H_
