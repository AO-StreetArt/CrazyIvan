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

#include <exception>
#include <string>

#ifndef SRC_MODEL_INCLUDE_SCENE_EXCEPTION_H_
#define SRC_MODEL_INCLUDE_SCENE_EXCEPTION_H_

// A Scene Exception

// A child class of std::exception
// which holds error information
struct SceneException: public std::exception {
  // An error message passed on initialization
  std::string int_msg;
  const char * int_msg_cstr;

  // Create a Neo4j Exception, and store the given error message
  inline SceneException(std::string msg) {
    int_msg = "Error in Scene: " + msg;
    int_msg_cstr = int_msg.c_str();
  }

  SceneException() {}
  ~SceneException() throw() {}
  // Show the error message in readable format
  const char * what() const throw() {
    return int_msg_cstr;
  }
};

#endif  // SRC_MODEL_INCLUDE_SCENE_EXCEPTION_H_
