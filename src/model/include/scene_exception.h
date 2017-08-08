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
