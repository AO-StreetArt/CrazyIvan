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

// This implements the Configuration Manager

// This takes in a Command Line Interpreter, and based on the options provided,
// decides how the application needs to be configured.  It may configure either
// from a configuration file, or from a Consul agent

#include <iostream>
#include <boost/cstdint.hpp>

#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"

#include "Poco/Util/ServerApplication.h"

#include "Poco/Logger.h"

#ifndef SRC_CONTROLLER_INCLUDE_HEARTBEAT_HANDLER_H_
#define SRC_CONTROLLER_INCLUDE_HEARTBEAT_HANDLER_H_

class HeartbeatHandler: public Poco::Net::HTTPRequestHandler {
 public:
  HeartbeatHandler() {}
  void handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response) {
    Poco::Logger::get("Controller").debug("Responding to Heartbeat Request");
    response.setChunkedTransferEncoding(true);
    response.setContentType("application/json");
    // Send the response
    std::ostream& ostr = response.send();
    std::string response_body_string = "{\"status\": \"up\"}";
    ostr << response_body_string;
    ostr.flush();
  }
};

#endif  // SRC_CONTROLLER_INCLUDE_HEARTBEAT_HANDLER_H_
