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

#include "include/message_processor.h"

// Register a device to a scene
// Predict a coordinate transform for the device
// and pass it back in the response
ProcessResult* \
  MessageProcessor::process_registration_message(SceneListInterface *obj_msg) {
  BaseMessageProcessor::logger().debug("Processing Registration Message");
  // Is this the first device being registered to the scene
  bool is_first_device = false;
  // Does the scene exist in the DB?
  bool does_scene_exist = true;
  // Is the device already registered to the scene
  bool already_registered = false;
  // Is the device already registered to another scene?
  bool previously_registered = false;
  // Current error information
  int current_err_code = NO_ERROR;
  std::string current_err_msg = "";

  // Basic checks to ensure we have the needed fields
  if (obj_msg->num_scenes() < 1) {
    current_err_code = INSUFF_DATA_ERROR;
    current_err_msg = "Scene needed to process registration";
  } else if (obj_msg->get_scene(0)->num_devices() < 1) {
    current_err_code = INSUFF_DATA_ERROR;
    current_err_msg = "Device needed to process registration";
  }

  // Determine if the scene exists in the DB
  if (current_err_code == NO_ERROR) {
    try {
      does_scene_exist = BaseMessageProcessor::get_query_helper()->\
        scene_exists(obj_msg->get_scene(0)->get_key());
    }
    catch (std::exception& e) {
      BaseMessageProcessor::logger().error("{\"Error\": \"%s\"", e.what());
      current_err_code = PROCESSING_ERROR;
      current_err_msg = e.what();
    }
  }

  // Build a new device key, if necessary
  std::string new_id;
  if (obj_msg->get_scene(0)->get_device(0)->get_key().empty()) {
    // Generate a new key
    BaseMessageProcessor::create_uuid(new_id);
    if (new_id.empty()) {
      BaseMessageProcessor::logger().error("Unknown error generating new key for scene");
      current_err_msg = "Error generating key for device";
      current_err_code = PROCESSING_ERROR;
    }
    BaseMessageProcessor::logger().information("Key Generated: %s", new_id);
  } else {
    new_id = obj_msg->get_scene(0)->get_device(0)->get_key();
  }

  // Determine if this is the first device being registered to the scene
  if (current_err_code == NO_ERROR) {
    if (!(does_scene_exist)) {
      is_first_device = true;
      // Create the scene
      BaseMessageProcessor::logger().debug("Creating Scene in database");
      ProcessResult *response = process_create_message(obj_msg);
      if (!(response->successful())) {
        BaseMessageProcessor::logger().error("Scene not found and creation failed");
        current_err_code = PROCESSING_ERROR;
        current_err_msg = "Scene not found and creation failed";
      }
      delete response;
    } else {
      try {
        already_registered = \
          BaseMessageProcessor::get_query_helper()->\
            is_ud_registered(obj_msg->get_scene(0)->get_key(), new_id);
      }
      catch (std::exception& e) {
        BaseMessageProcessor::logger().error("{\"Error\": \"%s\"", e.what());
        current_err_code = PROCESSING_ERROR;
        current_err_msg = e.what();
      }
    }
  }

  // Find other scenes that the device is registered to
  SceneListInterface *registered_scenes = NULL;
  if (current_err_code == NO_ERROR) {
    try {
      registered_scenes = \
        BaseMessageProcessor::get_query_helper()->\
          get_registrations(new_id);
    }
    catch (std::exception& e) {
      BaseMessageProcessor::logger().error("{\"Error\": \"%s\"", e.what());
      current_err_code = PROCESSING_ERROR;
      current_err_msg = e.what();
    }
  }
  if (!registered_scenes) {
    previously_registered = false;
  }

  // The transform matrix calculated for the given device
  // If we are registering the first device or cannot find
  // any paths from the device's previous scene(s), then we will
  // leave this as the identity matrix
  TransformInterface *new_transform = \
    BaseMessageProcessor::get_tfactory().build_transform();
  // Activate Transform Elements
  new_transform->translate(0, 0.0);
  new_transform->rotate(0, 0.0);

  // If we are not registering the first device, the scene exists,
  // the device is not already registered to the scene in question but
  // is registered to other scenes
  if (!(is_first_device) && does_scene_exist && !(already_registered) \
    && current_err_code == NO_ERROR && previously_registered) {
    BaseMessageProcessor::logger().debug("Attempting to find UD Transform");
    // Iterate through the scenes the device is already registered to
    // Try to find a path from these scenes to the current one in order
    // to establish a transform
    for (int i = 0; i < registered_scenes->num_scenes(); i++) {
      try {
        SceneTransformResult st_result = \
          BaseMessageProcessor::get_query_helper()->\
          calculate_scene_scene_transform(\
          registered_scenes->get_scene(i)->get_key(), \
          obj_msg->get_scene(0)->get_key());
        if (st_result.result_flag) {
          new_transform->add_transform(st_result.transform);
        }
      }
      catch (std::exception& e) {
        BaseMessageProcessor::logger().error("{\"Error\": \"%s\"", e.what());
        current_err_code = PROCESSING_ERROR;
        current_err_msg = e.what();
      }
    }
  }

  if (!(already_registered) && current_err_code == NO_ERROR) {
    // Create the registration link in the DB
    BaseMessageProcessor::logger().debug("Registering Device in the DB");
    try {
      BaseMessageProcessor::get_query_helper()->register_device_to_scene(\
        new_id, \
        obj_msg->get_scene(0)->get_key(), new_transform, previously_registered,\
        obj_msg->get_scene(0)->get_device(0)->get_connection_string(), \
        obj_msg->get_scene(0)->get_device(0)->get_hostname(), \
        obj_msg->get_scene(0)->get_device(0)->get_port());
    }
    catch (std::exception& e) {
      BaseMessageProcessor::logger().error("{\"Error\": \"%s\"", e.what());
      current_err_code = PROCESSING_ERROR;
      current_err_msg = e.what();
    }
  }

  // If we are registering to another scene and that scene is new, then
  // we can put in place a coordinate system transformation equal to the device
  // transform from the old scene, as the transform from the new scene is
  // the identity matrix
  if (previously_registered && is_first_device && \
    current_err_code == NO_ERROR) {
    BaseMessageProcessor::get_query_helper()->\
      process_UDUD_transformation(registered_scenes, obj_msg);
  }

  // Build a response
  BaseMessageProcessor::logger().debug("Creating Response message");
  std::string response_string;
  SceneListInterface *resp_interface = \
    BaseMessageProcessor::build_response_scene(SCENE_ENTER, current_err_code, \
    current_err_msg, obj_msg->get_transaction_id(), \
    obj_msg->get_scene(0)->get_key());

  // Build a Response Device
  UserDeviceInterface *ud_interface = NULL;
  if (new_transform && current_err_code == NO_ERROR) {
    ud_interface = BaseMessageProcessor::get_udfactory().build_device( \
      new_id, new_transform);

    resp_interface->get_scene(0)->add_device(ud_interface);
  }

  // Convert the Response to a string for sending
  resp_interface->to_msg_string(response_string);

  if (registered_scenes) {
    delete registered_scenes;
  }
  delete resp_interface;
  return new ProcessResult(response_string);
}

// Remove a device from a scene
// If we are removing the last user device from a scene,
// then remove the scene and try to move any paths that pass through the scene
ProcessResult* MessageProcessor::process_deregistration_message(\
  SceneListInterface *obj_msg) {
  BaseMessageProcessor::logger().debug("Processing Deregistration Message");
  // Current error information
  int current_err_code = NO_ERROR;
  std::string current_err_msg = "";

  // Remove the User Device
  if (current_err_code == NO_ERROR) {
    BaseMessageProcessor::logger().debug("Removing Device from Scene");
    try {
      BaseMessageProcessor::get_query_helper()->remove_device_from_scene(\
        obj_msg->get_scene(0)->get_device(0)->get_key(), \
        obj_msg->get_scene(0)->get_key());
    }
    catch (std::exception& e) {
      BaseMessageProcessor::logger().error("{\"Error\": \"%s\"", e.what());
      current_err_code = PROCESSING_ERROR;
      current_err_msg = e.what();
    }
  }
  // Build a response string
  BaseMessageProcessor::logger().debug("Creating Response message");
  std::string response_string;
  BaseMessageProcessor::build_string_response(SCENE_LEAVE, current_err_code, \
    current_err_msg, obj_msg->get_transaction_id(), \
    obj_msg->get_scene(0)->get_key(), response_string);

  return new ProcessResult(response_string);
}

// -------------------------------------------------------------------------- //
// ---------------------------Device Alignment------------------------------- //

// Align an objects transformation with a scene
// If the object is a member of other scenes:
//  -If a coordinate system transformation doesn't exist, then create one
//  -If a coordinate system transformation exists, then update it
ProcessResult* MessageProcessor::process_device_alignment_message(\
  SceneListInterface *obj_msg) {
  BaseMessageProcessor::logger().debug("Processing Alignment Message");
  // Current error information
  int current_err_code = NO_ERROR;
  std::string current_err_msg = "";

  bool registration_found = false;
  // Update the transformation between the scene and user device
  if (current_err_code == NO_ERROR) {
    try {
      registration_found = \
        BaseMessageProcessor::get_query_helper()->update_device_registration(\
        obj_msg->get_scene(0)->get_device(0)->get_key(), \
        obj_msg->get_scene(0)->get_key(), \
        obj_msg->get_scene(0)->get_device(0)->get_transform());
    }
    catch (std::exception& e) {
      BaseMessageProcessor::logger().error("{\"Error\": \"%s\"", e.what());
      current_err_code = PROCESSING_ERROR;
      current_err_msg = e.what();
    }
  }

  // If we could not find an existing registration to update
  // then we want to return a not found error code
  if (!registration_found) {
    BaseMessageProcessor::logger().debug("Existing Registration not found");
    current_err_code = NOT_FOUND;
    current_err_msg = "No Existing Registrations Found";
  }

  // Find scenes that the object is registered to
  SceneListInterface *registered_scenes = NULL;
  if (current_err_code == NO_ERROR) {
    BaseMessageProcessor::logger().debug("Retrieving Scenes already registered to");
    try {
      registered_scenes = \
        BaseMessageProcessor::get_query_helper()->\
        get_registrations(obj_msg->get_scene(0)->get_device(0)->get_key());
    }
    catch (std::exception& e) {
      BaseMessageProcessor::logger().error("{\"Error\": \"%s\"", e.what());
      current_err_code = PROCESSING_ERROR;
      current_err_msg = e.what();
    }
  }

  // Iterate through the scenes the devices is registered with &
  // Correct/Create scene-scene transforms
  if (current_err_code == NO_ERROR) {
    BaseMessageProcessor::logger().debug("Updating Scene-Scene Transforms");
    try {
      BaseMessageProcessor::get_query_helper()->\
        process_UDUD_transformation(registered_scenes, obj_msg);
    }
    catch (std::exception& e) {
      BaseMessageProcessor::logger().error("{\"Error\": \"%s\"", e.what());
      current_err_code = PROCESSING_ERROR;
      current_err_msg = e.what();
    }
  }

  // Build a protocol buffer response
  BaseMessageProcessor::logger().debug("Creating Response message");
  std::string response_string;
  BaseMessageProcessor::build_string_response(DEVICE_ALIGN, current_err_code, \
    current_err_msg, obj_msg->get_transaction_id(), \
    obj_msg->get_scene(0)->get_key(), response_string);

  if (registered_scenes) {
    delete registered_scenes;
  }

  return new ProcessResult(response_string);
}
