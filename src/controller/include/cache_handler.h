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

#include "model/include/scene_factory.h"
#include "model/include/scene_interface.h"

#include "api/include/scene_list_factory.h"
#include "api/include/scene_list_interface.h"

#include "cache/include/device_cache.h"

#include "aossl/core/include/kv_store_interface.h"

#include "rapidjson/document.h"
#include "rapidjson/error/en.h"

#include "app/include/ivan_utils.h"

#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"

#include "Poco/Util/ServerApplication.h"

#include "Poco/Logger.h"

#ifndef SRC_CONTROLLER_INCLUDE_CACHE_HANDLER_H_
#define SRC_CONTROLLER_INCLUDE_CACHE_HANDLER_H_

class CacheUpdateHandler: public Poco::Net::HTTPRequestHandler {
  AOSSL::KeyValueStoreInterface *config = NULL;
  std::string input_key;
  int mtype = -1;
  SceneListFactory scene_list_factory;
  SceneFactory scene_factory;
  DeviceCache *event_cache;
 public:
  CacheUpdateHandler(AOSSL::KeyValueStoreInterface *conf, DeviceCache *cache, int message_type, std::string& inp_key) \
    {config=conf;mtype=message_type;input_key.assign(inp_key);event_cache=cache;}
  void handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response) {
    Poco::Logger::get("Controller").debug("Responding to Cache Update Request");
    response.setChunkedTransferEncoding(true);
    response.setContentType("application/json");
    SceneListInterface *response_body = scene_list_factory.build_json_scene();
    // Try to update the cache
    response_body->set_msg_type(CACHE_DEL);
    SceneInterface *key_scn = scene_factory.build_scene();
    key_scn->set_key(input_key);
    response_body->add_scene(key_scn);
    if (mtype == CACHE_ADD) {
      event_cache->add_scene(input_key);
      response_body->set_err_code(NO_ERROR);
    } else if (mtype == CACHE_DEL) {
      event_cache->remove_scene(input_key);
      response_body->set_err_code(NO_ERROR);
    } else {
      response_body->set_err_code(BAD_MSG_TYPE_ERROR);
    }
    // Send the response
    std::ostream& ostr = response.send();
    std::string response_body_string;
    response_body->to_msg_string(response_body_string);
    ostr << response_body_string;
    ostr.flush();
    delete response_body;
  }
};

#endif  // SRC_CONTROLLER_INCLUDE_CACHE_HANDLER_H_
