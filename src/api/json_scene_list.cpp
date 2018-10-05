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

#include "include/json_scene_list.h"

// Create a JSON Scene List from a parsed Rapidjson Document
JsonSceneList::JsonSceneList(const rapidjson::Document& d) {
  SceneList::logger().information("Creating Scene from Rapidjson Document");
  if (d.IsObject()) {
    // Message Type
    rapidjson::Value::ConstMemberIterator msgtype_iter = \
      d.FindMember("msg_type");
    if (msgtype_iter != d.MemberEnd()) {
      SceneList::logger().debug("Message Type found");
      if (!(msgtype_iter->value.IsNull())) {
        set_msg_type(msgtype_iter->value.GetInt());
      }
    }
    // Operation Type
    rapidjson::Value::ConstMemberIterator optype_iter = \
      d.FindMember("operation");
    if (optype_iter != d.MemberEnd()) {
      SceneList::logger().debug("Operation Type found");
      if (!(optype_iter->value.IsNull())) {
        set_op_type(optype_iter->value.GetInt());
      }
    }
    // Transaction ID
    rapidjson::Value::ConstMemberIterator tranid_iter = \
      d.FindMember("transaction_id");
    if (tranid_iter != d.MemberEnd()) {
      SceneList::logger().debug("Transaction ID found");
      if (!(tranid_iter->value.IsNull())) {
        set_transaction_id(tranid_iter->value.GetString());
      }
    }
    // Encryption Key
    rapidjson::Value::ConstMemberIterator enckey_iter = \
      d.FindMember("encryption_key");
    if (enckey_iter != d.MemberEnd()) {
      SceneList::logger().debug("Encryption Key found");
      if (!(enckey_iter->value.IsNull())) {
        set_encryption_key(enckey_iter->value.GetString());
      }
    }
    // Encryption Salt
    rapidjson::Value::ConstMemberIterator encsalt_iter = \
      d.FindMember("encryption_salt");
    if (encsalt_iter != d.MemberEnd()) {
      SceneList::logger().debug("Encryption Salt found");
      if (!(encsalt_iter->value.IsNull())) {
        set_encryption_salt(encsalt_iter->value.GetString());
      }
    }
    // Decryption Key
    rapidjson::Value::ConstMemberIterator deckey_iter = \
      d.FindMember("decryption_key");
    if (deckey_iter != d.MemberEnd()) {
      SceneList::logger().debug("Decryption Key found");
      if (!(deckey_iter->value.IsNull())) {
        set_decryption_key(deckey_iter->value.GetString());
      }
    }
    // Decryption Salt
    rapidjson::Value::ConstMemberIterator decsalt_iter = \
      d.FindMember("decryption_salt");
    if (decsalt_iter != d.MemberEnd()) {
      SceneList::logger().debug("Decryption Salt found");
      if (!(decsalt_iter->value.IsNull())) {
        set_decryption_salt(decsalt_iter->value.GetString());
      }
    }
    // Event Destination Host
    rapidjson::Value::ConstMemberIterator edhkey_iter = \
      d.FindMember("event_destination_host");
    if (edhkey_iter != d.MemberEnd()) {
      SceneList::logger().debug("Event Destination Host found");
      if (!(edhkey_iter->value.IsNull())) {
        set_event_destination_host(edhkey_iter->value.GetString());
      }
    }
    // Event Destination Port
    rapidjson::Value::ConstMemberIterator edpsalt_iter = \
      d.FindMember("event_destination_port");
    if (edpsalt_iter != d.MemberEnd()) {
      SceneList::logger().debug("Event Destination Port found");
      if (!(edpsalt_iter->value.IsNull())) {
        set_event_destination_port(edpsalt_iter->value.GetInt());
      }
    }
    // Error Code
    rapidjson::Value::ConstMemberIterator errcode_iter = \
      d.FindMember("err_code");
    if (errcode_iter != d.MemberEnd()) {
      SceneList::logger().debug("Error Code found");
      if (!(errcode_iter->value.IsNull())) {
        set_err_code(errcode_iter->value.GetInt());
      }
    }
    // Error Message
    rapidjson::Value::ConstMemberIterator errmsg_iter = \
      d.FindMember("err_msg");
    if (errmsg_iter != d.MemberEnd()) {
      SceneList::logger().debug("Error Message found");
      if (!(errmsg_iter->value.IsNull())) {
        set_err_msg(errmsg_iter->value.GetString());
      }
    }
    // Query Maximum Number of Records
    rapidjson::Value::ConstMemberIterator maxrcrd_iter = \
      d.FindMember("num_records");
    if (maxrcrd_iter != d.MemberEnd()) {
      SceneList::logger().debug("Max Records found");
      if (maxrcrd_iter->value.IsInt()) {
        set_num_records(maxrcrd_iter->value.GetInt());
      }
    }
    // Query Start Record
    rapidjson::Value::ConstMemberIterator startrcrd_iter = \
      d.FindMember("start_record");
    if (startrcrd_iter != d.MemberEnd()) {
      SceneList::logger().debug("Start Record found");
      if (startrcrd_iter->value.IsInt()) {
        set_start_record(startrcrd_iter->value.GetInt());
      }
    }
    // Scene List
    if (d.HasMember("scenes")) {
      SceneList::logger().debug("Scene List found");
      const rapidjson::Value& scenes_val = d["scenes"];
      if (scenes_val.IsArray()) {
        for (auto& itr : scenes_val.GetArray()) {
          SceneInterface *scd = sfactory.build_scene();
          // Scene Key
          rapidjson::Value::ConstMemberIterator key_iter = \
          itr.FindMember("key");
          if (key_iter != itr.MemberEnd()) {
            SceneList::logger().debug("Key found");
            if (!(key_iter->value.IsNull())) {
              scd->set_key(key_iter->value.GetString());
            }
          } else {SceneList::logger().debug("Key not found");}
          // Scene Name
          rapidjson::Value::ConstMemberIterator name_iter = \
            itr.FindMember("name");
          if (name_iter != itr.MemberEnd()) {
            SceneList::logger().debug("Name found");
            if (!(name_iter->value.IsNull())) {
              scd->set_name(name_iter->value.GetString());
            }
          }
          // Is Scene Active
          rapidjson::Value::ConstMemberIterator active_iter = \
            itr.FindMember("active");
          if (active_iter != itr.MemberEnd()) {
            SceneList::logger().debug("Active flag found");
            if (!(active_iter->value.IsNull())) {
              scd->set_active(active_iter->value.GetBool());
            }
          }
          rapidjson::Value::ConstMemberIterator region_iter = \
            itr.FindMember("region");
          if (region_iter != itr.MemberEnd()) {
            SceneList::logger().debug("Region found");
            if (!(region_iter->value.IsNull())) {
              scd->set_region(region_iter->value.GetString());
            }
          }
          // Scene Latitude
          rapidjson::Value::ConstMemberIterator lat_iter = \
          itr.FindMember("latitude");
          if (lat_iter != itr.MemberEnd()) {
            SceneList::logger().debug("Latitude found");
            if (!(lat_iter->value.IsNull())) {
              scd->set_latitude(lat_iter->value.GetDouble());
            }
          }
          // Scene Longitude
          rapidjson::Value::ConstMemberIterator long_iter = \
            itr.FindMember("longitude");
          if (long_iter != itr.MemberEnd()) {
            SceneList::logger().debug("Longitude found");
            if (!(long_iter->value.IsNull())) {
              scd->set_longitude(long_iter->value.GetDouble());
            }
          }
          // Query Distance
          rapidjson::Value::ConstMemberIterator dist_iter = \
            itr.FindMember("distance");
          if (dist_iter != itr.MemberEnd()) {
            SceneList::logger().debug("Distance found");
            if (!(dist_iter->value.IsNull())) {
              scd->set_distance(dist_iter->value.GetDouble());
            }
          }

          // Scene Assets
          rapidjson::Value::ConstMemberIterator assets_itr = \
            itr.FindMember("assets");
          if (assets_itr != itr.MemberEnd()) {
            SceneList::logger().debug("Scene Assets found");
            if (!(assets_itr->value.IsNull())) {
              for (auto& asset_itr : assets_itr->value.GetArray()) {
                scd->add_asset(asset_itr.GetString());
              }
            }
          }

          // Scene Tags
          rapidjson::Value::ConstMemberIterator tags_itr = \
            itr.FindMember("tags");
          if (tags_itr != itr.MemberEnd()) {
            SceneList::logger().debug("Tags found");
            if (!(tags_itr->value.IsNull())) {
              for (auto& tag_itr : tags_itr->value.GetArray()) {
                scd->add_tag(tag_itr.GetString());
              }
            }
          }

          // Process Scene Transform
          if (itr.HasMember("transform")) {
            SceneList::logger().debug("Transform found");
            TransformInterface *new_scene_transform = \
              SceneList::create_transform();
            const rapidjson::Value& scene_trn_val = itr["transform"];
            if (!(scene_trn_val.IsNull())) {
              for (
                rapidjson::Value::ConstMemberIterator scn_trns_itr = \
                scene_trn_val.MemberBegin();
                scn_trns_itr != scene_trn_val.MemberEnd();
                ++scn_trns_itr
              ) {
                const char * name_cstring = scn_trns_itr->name.GetString();
                SceneList::logger().debug("Processing Transform Member: %s", name_cstring);
                if (strcmp(name_cstring, "translation") == 0) {
                  SceneList::logger().debug("Translation found");
                  const rapidjson::Value& translation_val = scn_trns_itr->value;
                  int i = 0;
                  for (auto& translation_itr : translation_val.GetArray()) {
                    new_scene_transform->translate(i, \
                      translation_itr.GetDouble());
                    ++i;
                  }
                } else if (strcmp(name_cstring, "rotation") == 0) {
                  SceneList::logger().debug("Rotation found");
                  const rapidjson::Value& rotation_val = scn_trns_itr->value;
                  int i = 0;
                  for (auto& rotation_itr : rotation_val.GetArray()) {
                    new_scene_transform->rotate(i, rotation_itr.GetDouble());
                    ++i;
                  }
                }
              }
            }
            scd->set_transform(new_scene_transform);
          }

          // Process User Device List, including transforms
          if (itr.HasMember("devices")) {
            SceneList::logger().debug("Device List found");
            const rapidjson::Value& device_list_val = itr["devices"];
            if (!(device_list_val.IsNull())) {
              for (auto& device_itr : device_list_val.GetArray()) {
                // Process the device key
                rapidjson::Value::ConstMemberIterator ud_key_iter = \
                device_itr.FindMember("key");
                if (ud_key_iter != device_itr.MemberEnd()) {
                  if (!(ud_key_iter->value.IsNull())) {
                    SceneList::logger().debug("UD Key found");
                    UserDeviceInterface *new_device = \
                      SceneList::create_device(ud_key_iter->value.GetString());

                    // Process device connectivity information
                    rapidjson::Value::ConstMemberIterator ud_conn_iter = \
                      device_itr.FindMember("connection_string");
                    if (ud_conn_iter != device_itr.MemberEnd()) {
                      if (!(ud_conn_iter->value.IsNull())) {
                        new_device->set_connection_string(ud_conn_iter->value.GetString());
                      }
                    }

                    rapidjson::Value::ConstMemberIterator ud_host_iter = \
                      device_itr.FindMember("hostname");
                    if (ud_host_iter != device_itr.MemberEnd()) {
                      if (!(ud_host_iter->value.IsNull())) {
                        new_device->set_hostname(ud_host_iter->value.GetString());
                      }
                    }

                    rapidjson::Value::ConstMemberIterator ud_port_iter = \
                      device_itr.FindMember("port");
                    if (ud_port_iter != device_itr.MemberEnd()) {
                      if (!(ud_port_iter->value.IsNull())) {
                        new_device->set_port(ud_port_iter->value.GetInt());
                      }
                    }

                    // Process the device transform
                    if (device_itr.HasMember("transform")) {
                      SceneList::logger().debug("UD Transform found");
                      TransformInterface *new_ud_transform = \
                        SceneList::create_transform();
                      const rapidjson::Value& ud_trn_val = device_itr["transform"];
                      if (!(ud_trn_val.IsNull())) {
                        for (rapidjson::Value::ConstMemberIterator ud_trns_itr = \
                          ud_trn_val.MemberBegin(); \
                          ud_trns_itr != ud_trn_val.MemberEnd(); ++ud_trns_itr) {
                            const char * udname_cstring = \
                              ud_trns_itr->name.GetString();
                            SceneList::logger().debug("Processing Transform Member: %s", udname_cstring);
                            if (strcmp(udname_cstring, "translation") == 0) {
                              SceneList::logger().debug("Translation found");
                              const rapidjson::Value& udtranslation_val = \
                                ud_trns_itr->value;
                              if (!(udtranslation_val.IsNull())) {
                                int i = 0;
                                for (auto& translation_itr : \
                                  udtranslation_val.GetArray()) {
                                  new_ud_transform->translate(i, \
                                    translation_itr.GetDouble());
                                  ++i;
                                }
                              }
                            } else if (strcmp(udname_cstring, "rotation") == 0) {
                              SceneList::logger().debug("Rotation found");
                              const rapidjson::Value& udrotation_val = \
                              ud_trns_itr->value;
                              if (!(udrotation_val.IsNull())) {
                                int i = 0;
                                for (auto& rotation_itr : udrotation_val.GetArray()) {
                                  new_ud_transform->rotate(i, rotation_itr.GetDouble());
                                  ++i;
                                }
                              }
                            }
                          }
                        }
                        new_device->set_transform(new_ud_transform);
                      }
                      scd->add_device(new_device);
                    }
                  }
                }
              }
            }

            add_scene(scd);
          }
        }
      }
    } else {SceneList::logger().error("JSON Message detected that is not an object");}
  }

  void JsonSceneList::to_msg_string(std::string &out_string) {
    SceneList::logger().information("To JSON message Called on Scene");
    // Initialize the string buffer and writer
    rapidjson::StringBuffer s;
    rapidjson::Writer<rapidjson::StringBuffer> writer(s);

    // Start writing the object
    // Syntax taken directly from
    // simplewriter.cpp in rapidjson examples

    writer.StartObject();

    // Add the message type
    writer.Key("msg_type");
    writer.Uint(get_msg_type());
    // Add the error code
    writer.Key("err_code");
    writer.Uint(get_err_code());
    // Add the error message
    if (!get_err().empty()) {
      writer.Key("err_msg");
      writer.String(get_err().c_str(), (rapidjson::SizeType)get_err().length());
    }
    // Add the Transaction ID
    if (!(get_transaction_id().empty())) {
      writer.Key("transaction_id");
      writer.String(get_transaction_id().c_str(), \
      (rapidjson::SizeType)get_transaction_id().length());
    }
    // Add the Encryption Key
    if (!(get_encryption_key().empty())) {
      writer.Key("encryption_key");
      writer.String(get_encryption_key().c_str(), \
      (rapidjson::SizeType)get_encryption_key().length());
    }
    // Add the Encryption Salt
    if (!(get_encryption_salt().empty())) {
      writer.Key("encryption_salt");
      writer.String(get_encryption_salt().c_str(), \
      (rapidjson::SizeType)get_encryption_salt().length());
    }
    // Add the Decryption Key
    if (!(get_decryption_key().empty())) {
      writer.Key("decryption_key");
      writer.String(get_decryption_key().c_str(), \
      (rapidjson::SizeType)get_decryption_key().length());
    }
    // Add the Decryption Salt
    if (!(get_decryption_salt().empty())) {
      writer.Key("decryption_salt");
      writer.String(get_decryption_salt().c_str(), \
      (rapidjson::SizeType)get_decryption_salt().length());
    }
    // Add the Event Destination Host
    if (!(get_event_destination_host().empty())) {
      writer.Key("event_destination_host");
      writer.String(get_event_destination_host().c_str(), \
      (rapidjson::SizeType)get_event_destination_host().length());
    }
    // Add the Event Destination Port
    if (get_event_destination_port() > 0) {
      writer.Key("event_destination_port");
      writer.Int(get_event_destination_port());
    }
    // Determine how many scenes we want to return in the message
    int num_return_scenes = num_scenes();
    if ((get_num_records() != -1) && (get_num_records() < num_return_scenes)) {
      num_return_scenes = get_num_records();
    }
    writer.Key("num_records");
    writer.Uint(num_return_scenes);
    writer.Key("start_record");
    writer.Uint(get_start_record());
    // Add the scene list
    writer.Key("scenes");
    writer.StartArray();
    // Iterate through the data list and process each scene
    for (int a = 0; a < num_return_scenes; a++) {
      writer.StartObject();

      // Add the key
      if (!(get_scene(a)->get_key().empty())) {
        SceneList::logger().debug("Writing key to JSON: %s -- %d", \
          get_scene(a)->get_key(), \
          static_cast<int>(get_scene(a)->get_key().length()));
        writer.Key("key");
        writer.String(get_scene(a)->get_key().c_str(), \
        (rapidjson::SizeType)get_scene(a)->get_key().length());
      }
      // Add the name
      if (!(get_scene(a)->get_name().empty())) {
        writer.Key("name");
        writer.String(get_scene(a)->get_name().c_str(), \
        (rapidjson::SizeType)get_scene(a)->get_name().length());
      }
      // Add the region
      if (!(get_scene(a)->get_region().empty())) {
        writer.Key("region");
        writer.String(get_scene(a)->get_region().c_str(), \
        (rapidjson::SizeType)get_scene(a)->get_region().length());
      }
      // Add the latitude
      if (get_scene(a)->get_latitude() > -9999.0) {
        writer.Key("latitude");
        writer.Double(get_scene(a)->get_latitude());
      }
      // Add the active flag
      writer.Key("active");
      writer.Bool(get_scene(a)->active());
      // Add the longitude
      if (get_scene(a)->get_longitude() > -9999.0) {
        writer.Key("longitude");
        writer.Double(get_scene(a)->get_longitude());
      }
      // Add the distance
      if (get_scene(a)->get_distance() > -9999.0) {
        writer.Key("distance");
        writer.Double(get_scene(a)->get_distance());
      }

      // Asset IDs
      writer.Key("assets");
      writer.StartArray();

      for (int m = 0; m < get_scene(a)->num_assets(); m++) {
        writer.String(get_scene(a)->get_asset(m).c_str(), \
        (rapidjson::SizeType)get_scene(a)->get_asset(m).length());
      }

      writer.EndArray();

      // Tags
      writer.Key("tags");
      writer.StartArray();

      for (int n = 0; n < get_scene(a)->num_tags(); n++) {
        writer.String(get_scene(a)->get_tag(n).c_str(), \
        (rapidjson::SizeType)get_scene(a)->get_tag(n).length());
      }

      writer.EndArray();

      // Add the scene transform
      if (get_scene(a)->has_transform()) {
        writer.Key("transform");
        writer.StartObject();
        if (get_scene(a)->get_scene_transform()->has_translation()) {
          // Add the translation
          writer.Key("translation");
          writer.StartArray();
          writer.Double(get_scene(a)->get_scene_transform()->translation(0));
          writer.Double(get_scene(a)->get_scene_transform()->translation(1));
          writer.Double(get_scene(a)->get_scene_transform()->translation(2));
          writer.EndArray();
        }
        if (get_scene(a)->get_scene_transform()->has_rotation()) {
          // Add the rotation
          writer.Key("rotation");
          writer.StartArray();
          writer.Double(get_scene(a)->get_scene_transform()->rotation(0));
          writer.Double(get_scene(a)->get_scene_transform()->rotation(1));
          writer.Double(get_scene(a)->get_scene_transform()->rotation(2));
          writer.EndArray();
        }
        writer.EndObject();
      }

      // Add the device list
      writer.Key("devices");
      writer.StartArray();

      // Iterate through the device list for the scene
      for (int b = 0; b < get_scene(a)->num_devices(); b++) {
        writer.StartObject();
        // Add the key
        if (!(get_scene(a)->get_device(b)->get_key().empty())) {
          writer.Key("key");
          writer.String(get_scene(a)->get_device(b)->get_key().c_str(), \
          (rapidjson::SizeType)get_scene(a)->get_device(b)->get_key().length());
        }
        // Add the connectivity information
        if (!(get_scene(a)->get_device(b)->get_connection_string().empty())) {
          writer.Key("connection_string");
          writer.String(get_scene(a)->get_device(b)->get_connection_string().c_str(), \
          (rapidjson::SizeType)get_scene(a)->get_device(b)->get_connection_string().length());
        }
        if (!(get_scene(a)->get_device(b)->get_hostname().empty())) {
          writer.Key("hostname");
          writer.String(get_scene(a)->get_device(b)->get_hostname().c_str(), \
          (rapidjson::SizeType)get_scene(a)->get_device(b)->get_hostname().length());
        }
        if (get_scene(a)->get_device(b)->get_port() < 999999) {
          writer.Key("port");
          writer.Uint(get_scene(a)->get_device(b)->get_port());
        }
        // Add the device transform
        if (get_scene(a)->get_device(b)->has_transform()) {
          writer.Key("transform");
          writer.StartObject();
          if (get_scene(a)->get_device(b)->get_transform()->has_translation()) {
            // Add the translation
            writer.Key("translation");
            writer.StartArray();
            writer.Double(get_scene(a)->get_device(b)->\
              get_transform()->translation(0));
            writer.Double(get_scene(a)->get_device(b)->\
              get_transform()->translation(1));
            writer.Double(get_scene(a)->get_device(b)->\
              get_transform()->translation(2));
            writer.EndArray();
          }
          if (get_scene(a)->get_device(b)->get_transform()->has_rotation()) {
            // Add the rotation
            writer.Key("rotation");
            writer.StartArray();
            writer.Double(get_scene(a)->get_device(b)->\
              get_transform()->rotation(0));
            writer.Double(get_scene(a)->get_device(b)->\
              get_transform()->rotation(1));
            writer.Double(get_scene(a)->get_device(b)->\
              get_transform()->rotation(2));
            writer.EndArray();
          }
          writer.EndObject();
        }
        writer.EndObject();
      }

      writer.EndArray();

      writer.EndObject();
    }
    writer.EndArray();

    writer.EndObject();

    // The Stringbuffer now contains a json message
    // of the object
    out_string.assign(s.GetString());
  }
