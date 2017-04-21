#include "scene.h"

void Transform::invert() {
  tran[0] = (-1.0) * tran[0];
  tran[1] = (-1.0) * tran[1];
  tran[2] = (-1.0) * tran[2];
  rot[0] = (-1.0) * rot[0];
  rot[1] = (-1.0) * rot[1];
  rot[2] = (-1.0) * rot[2];
}

Transform::Transform() {
  tran_flag=false;
  rot_flag=false;
  tran.push_back(0.0);
  tran.push_back(0.0);
  tran.push_back(0.0);
  rot.push_back(0.0);
  rot.push_back(0.0);
  rot.push_back(0.0);
}

Transform::Transform(protoScene::SceneList_Transformation data) {

  obj_logging->debug("Converting Transform Data from Protocol Buffer");

  tran_flag=false;
  rot_flag=false;
  tran.push_back(0.0);
  tran.push_back(0.0);
  tran.push_back(0.0);
  rot.push_back(0.0);
  rot.push_back(0.0);
  rot.push_back(0.0);
  if (data.has_translation()) {
    protoScene::SceneList_Vertex3 ptrans = data.translation();
    tran[0] = ptrans.x();
    tran[1] = ptrans.y();
    tran[2] = ptrans.z();
    tran_flag=true;
    obj_logging->debug("Translation added");
  }
  if (data.has_rotation()) {
    protoScene::SceneList_Vertex3 prot = data.rotation();
    rot[0] = prot.x();
    rot[1] = prot.y();
    rot[2] = prot.z();
    rot_flag=true;
    obj_logging->debug("Rotation added");
  }
}

Transform::Transform(const rapidjson::Value *val) {
  obj_logging->debug("Converting Transform Data from Rapidjson Value");

  tran_flag=false;
  rot_flag=false;
  tran.push_back(0.0);
  tran.push_back(0.0);
  tran.push_back(0.0);
  rot.push_back(0.0);
  rot.push_back(0.0);
  rot.push_back(0.0);

  if (val->HasMember("translation")) {
    translation_val = &((*val)["translation"]);
    if (translation_val->IsArray()) {
      if (translation_val->Size() == 3) {
        obj_logging->debug("Translation List found");
        tran[0] = translation_val[0].GetDouble();
        tran[1] = translation_val[1].GetDouble();
        tran[2] = translation_val[2].GetDouble();
        tran_flag=true;
      }
      else {obj_logging->error("Translation array does not have 3 elements");}
    }
    else {obj_logging->error("Translation element is not an array");}
  }
  if (val->HasMember("rotation")) {
    rotation_val = &((*val)["rotation"]);
    if (rotation_val->IsArray()) {
      if (rotation_val->Size() == 3) {
        obj_logging->debug("Rotation List found");
        rot[0] = rotation_val[0].GetDouble();
        rot[1] = rotation_val[1].GetDouble();
        rot[2] = rotation_val[2].GetDouble();
        rot_flag=true;
      }
      else {obj_logging->error("Rotation array does not have 3 elements");}
    }
    else {obj_logging->error("Rotation element is not an array");}
  }
}

void Transform::add_transform(Transform &t, bool inverted) {
  if (t.has_translation()) {
    if (inverted) {
      tran[0] = tran[0] - t.translation(0);
      tran[1] = tran[1] - t.translation(1);
      tran[2] = tran[2] - t.translation(2);
    }
    else {
      tran[0] = tran[0] + t.translation(0);
      tran[1] = tran[1] + t.translation(1);
      tran[2] = tran[2] + t.translation(2);
    }
    tran_flag=true;
  }
  if (t.has_rotation()) {
    if (inverted) {
      rot[0] = rot[0] - t.rotation(0);
      rot[1] = rot[1] - t.rotation(1);
      rot[2] = rot[2] - t.rotation(2);
    }
    else {
      rot[0] = rot[0] + t.rotation(0);
      rot[1] = rot[1] + t.rotation(1);
      rot[2] = rot[2] + t.rotation(2);
    }
    rot_flag=true;
  }
}

//Create a new user device object from a protocol buffer user device object
UserDevice::UserDevice(protoScene::SceneList_UserDevice ud_data) {

  obj_logging->debug("Converting User Device Data from Protocol Buffer");

  if (ud_data.has_key()) {
    key = ud_data.key();
    obj_logging->debug(key);
  }
  if (ud_data.has_transform()) {
    trans = new Transform ( ud_data.transform() );
		trns_flag=true;
    obj_logging->debug("Device Transform added");
  }
}

UserDevice::UserDevice(const rapidjson::Value& val) {
  if (val.IsObject()) {
    obj_logging->info("Creating User Device from Rapidjson Value");
    if (val.HasMember("key")) {
      obj_logging->debug("Key found");
      key_val = &val["key"];
      key = key_val->GetString();
      obj_logging->debug(key);
    }
    if (val.HasMember("transform")) {
      obj_logging->debug("Transform found");
      transform_val = &val["transform"];
      trans = new Transform(transform_val);
    }
  }
}

//Copy Constructor
UserDevice::UserDevice(const UserDevice &ud) {
  key=ud.get_key();
  trns_flag=false;
  if ( ud.has_transform() ) {
    trns_flag=true;
    trans = new Transform;
    for (int i=0;i<3;i++) {
      if (ud.get_transform()->has_translation()) {
        trans->translate(i, ud.get_translation(i));
      }
      if (ud.get_transform()->has_rotation()) {
        trans->rotate(i, ud.get_rotation(i));
      }
    }
  }
}

//Copy Constructor
SceneData::SceneData(const SceneData& sd) {
  key=sd.get_key();
  name=sd.get_name();
  latitude=sd.get_latitude();
  longitude=sd.get_longitude();
  distance=sd.get_distance();
  trns_flag=false;
  if (sd.has_transform()) {
    trns_flag=true;
    scene_transform = new Transform;
    for (int i=0;i<3;i++) {
      scene_transform->translate(i, sd.get_scene_transform()->translation(i));
      scene_transform->rotate(i, sd.get_scene_transform()->rotation(i));
    }
  }
  for (int j=0;j<sd.num_devices();j++) {
    Transform *new_tran = new Transform;
    for (int k=0;k<3;k++) {
      if ( sd.get_device(j)->has_transform() ) {
        new_tran->translate(k, sd.get_device(j)->get_translation(k));
        new_tran->rotate(k, sd.get_device(j)->get_rotation(k));
      }
    }
    UserDevice *new_dev = new UserDevice (sd.get_device(j)->get_key(), new_tran);
    devices.push_back(new_dev);
  }
}

//Create a new scene data object from a protocol buffer scene object
SceneData::SceneData(protoScene::SceneList_Scene scn_data) {

  obj_logging->debug("Converting Scene Data from Protocol Buffer");

  //New variables
  trns_flag=false;

  //Perform the translation
  if (scn_data.has_key()) {
    key = scn_data.key();
    obj_logging->debug(key);
  }
  if (scn_data.has_latitude()) {
		latitude = scn_data.latitude();
    obj_logging->debug(latitude);
	}
  else {latitude = -9999.0;}
  if (scn_data.has_longitude()) {
		longitude = scn_data.longitude();
    obj_logging->debug(longitude);
	}
  else {longitude = -9999.0;}
  if (scn_data.has_distance()) {
		distance = scn_data.distance();
    obj_logging->debug(distance);
	}
  else {distance = -1.0;}
  if (scn_data.has_name()) {
		name = scn_data.name();
    obj_logging->debug(name);
	}
  if (scn_data.has_transform()) {
    scene_transform = new Transform ( scn_data.transform() );
		trns_flag=true;
    obj_logging->debug("Transform added");
  }
  if (scn_data.devices_size() > 0) {
    for (int k=0; k< scn_data.devices_size(); k++) {
      UserDevice *ud = new UserDevice (scn_data.devices(k));
      devices.push_back(ud);
      obj_logging->debug("User Device added");
    }
  }
}

SceneData::SceneData(const rapidjson::Value& val) {
  obj_logging->info("Creating Scene Data from Rapidjson Value");
  if (val.IsObject()) {
    if (val.HasMember("key")) {
      obj_logging->debug("Key found");
      key_val = &val["key"];
      key = key_val->GetString();
      obj_logging->debug(key);
    }
    if (val.HasMember("name")) {
      obj_logging->debug("Name found");
      name_val = &val["name"];
      name = name_val->GetString();
      obj_logging->debug(name);
		}
    if (val.HasMember("latitude")) {
      obj_logging->debug("Latitude found");
      lat_val = &val["latitude"];
      latitude = lat_val->GetDouble();
      obj_logging->debug(latitude);
		}
    else {latitude = -9999.0;}
    if (val.HasMember("longitude")) {
      obj_logging->debug("Longitude found");
      long_val = &val["longitude"];
      longitude = long_val->GetDouble();
      obj_logging->debug(latitude);
		}
    else {longitude = -9999.0;}
    if (val.HasMember("distance")) {
      obj_logging->debug("Distance found");
      dist_val = &val["distance"];
      distance = dist_val->GetDouble();
      obj_logging->debug(distance);
		}
    else {distance = -9999.0;}
    if (val.HasMember("transform")) {
      obj_logging->debug("Transform found");
      transform_val = &val["transform"];
      scene_transform = new Transform(transform_val);
    }
    if (val.HasMember("devices")) {
      devices_val = &val["devices"];
      if (devices_val->IsArray()) {
        obj_logging->debug("Device List found");
        for (rapidjson::SizeType i = 0; i < devices_val->Size();i++) {
          devices.push_back(new UserDevice(devices_val[i]));
        }
      }
    }
  }

}

//Constructor accepting Protocol Buffer
Scene::Scene(protoScene::SceneList buffer) {

  obj_logging->debug("Starting Conversion from Protocol Buffer");

  //Perform the conversion
  if (buffer.has_message_type()) {
		msg_type = buffer.message_type();
    obj_logging->debug(msg_type);
	}
  else {msg_type=-1;}
  if (buffer.has_transaction_id()) {
    transaction_id = buffer.transaction_id();
    obj_logging->debug(transaction_id);
  }
  else {transaction_id = "";}
  if (buffer.has_err_msg()) {
    err_msg = buffer.err_msg();
    obj_logging->debug(err_msg);
  }
  else {err_msg = "";}
  if (buffer.has_err_code()) {
    err_code = buffer.err_code();
    obj_logging->debug(err_code);
  }
  else {err_code=100;}
  if (buffer.has_num_records()) {
    num_records = buffer.num_records();
    obj_logging->debug(num_records);
  }
  else {num_records=10;}
  for (int a = 0; a < buffer.scenes_size(); a++) {
    SceneData *sc_data = new SceneData (buffer.scenes(a));
    data.push_back(sc_data);
    obj_logging->debug("Scene added");
  }
}

//Convert to Protocol Buffer message
std::string Scene::to_protobuf() {

  //Build the new protocol buffer object
	protoScene::SceneList *new_proto = NULL;
  new_proto = new protoScene::SceneList;

  //Convert basic elements
	if (msg_type != -1) {
		new_proto->set_message_type(msg_type);
	}
	if ( !(transaction_id.empty()) ) {
		new_proto->set_transaction_id(transaction_id);
	}
  if ( !(err_msg.empty()) ) {
		new_proto->set_err_msg(err_msg);
	}
	new_proto->set_err_code(err_code);

  //Determine how many scenes we want to return in the message
  int num_scenes = static_cast<int>(data.size());
  if ( (num_records != -1) && (num_records < num_scenes) ) {
    num_scenes = num_records;
  }
  //Iterate through the data list and process each scene
  for (int a = 0; a < num_scenes; a++) {
    //Build a new scene object
    protoScene::SceneList_Scene *scn = new_proto->add_scenes();

    //Name & Key
    std::string key = data[a]->get_key();
    obj_logging->info("Scene:To Proto message Called on object");
  	obj_logging->info(key);
    if ( !(key.empty()) ) {
  		scn->set_key(key);
  	}
    std::string name = data[a]->get_name();
    if ( !(name.empty()) ) {
  		scn->set_name(name);
  	}

    //Lat/long
    scn->set_latitude(data[a]->get_latitude());
  	scn->set_longitude(data[a]->get_longitude());

    //distance
    if (data[a]->get_distance() >= 0.0) {
      scn->set_distance(data[a]->get_distance());
    }

    //Convert transform
    if (data[a]->has_transform()) {
      protoScene::SceneList_Transformation *trn = scn->mutable_transform();
      protoScene::SceneList_Vertex3 *proto_translation = trn->mutable_translation();
      protoScene::SceneList_Vertex3 *proto_rot = trn->mutable_rotation();
      proto_translation->set_x(data[a]->get_scene_transform()->translation(0));
      proto_translation->set_y(data[a]->get_scene_transform()->translation(1));
      proto_translation->set_z(data[a]->get_scene_transform()->translation(2));
      proto_rot->set_x(data[a]->get_scene_transform()->rotation(0));
      proto_rot->set_y(data[a]->get_scene_transform()->rotation(1));
      proto_rot->set_z(data[a]->get_scene_transform()->rotation(2));
    }

    //Iterate through the device list for the scene and write those
    for (int b = 0; b < data[a]->num_devices(); b++) {
      protoScene::SceneList_UserDevice *ud = scn->add_devices();
      std::string key = data[a]->get_device(b)->get_key();
      if ( !(key.empty()) ) {
    		ud->set_key(key);
    	}

      if (data[a]->get_device(b)->has_transform()) {
        protoScene::SceneList_Transformation *trn = ud->mutable_transform();
        protoScene::SceneList_Vertex3 *proto_translation = trn->mutable_translation();
        protoScene::SceneList_Vertex3 *proto_rot = trn->mutable_rotation();
        proto_translation->set_x(data[a]->get_device(b)->get_transform()->translation(0));
        proto_translation->set_y(data[a]->get_device(b)->get_transform()->translation(1));
        proto_translation->set_z(data[a]->get_device(b)->get_transform()->translation(2));
        proto_rot->set_x(data[a]->get_device(b)->get_transform()->rotation(0));
        proto_rot->set_y(data[a]->get_device(b)->get_transform()->rotation(1));
        proto_rot->set_z(data[a]->get_device(b)->get_transform()->rotation(2));
      }
    }
  }

  //Serialize into a std::string
  new_proto->SerializeToString(&protobuf_string);
	obj_logging->debug("Protocol Buffer Serialized to String");
	obj_logging->debug(protobuf_string);
  if (new_proto) {
	   delete new_proto;
  }
	return protobuf_string;
}

Scene::Scene(const rapidjson::Document& d) {
  obj_logging->info("Creating Scene from Rapidjson Document");
  if (d.IsObject()) {
    if (d.HasMember("msg_type")) {
      obj_logging->debug("Message Type found");
      ms_type_val = &d["msg_type"];
      msg_type = ms_type_val->GetInt();
    }
    if (d.HasMember("transaction_id")) {
      obj_logging->debug("Transaction ID found");
      tran_id_val = &d["transaction_id"];
      transaction_id = tran_id_val->GetString();
		}
    if (d.HasMember("err_code")) {
      obj_logging->debug("Error Code found");
      err_code_val = &d["err_code"];
      err_code = err_code_val->GetInt();
    }
    if (d.HasMember("err_msg")) {
      obj_logging->debug("Error Message found");
      err_str_val = &d["err_msg"];
      err_msg = err_str_val->GetString();
		}
    if (d.HasMember("num_records")) {
      obj_logging->debug("Max Records found");
      records_val = &d["num_records"];
      num_records = records_val->GetInt();
    }
    if (d.HasMember("scenes")) {
      obj_logging->debug("Scene List found");
      scenes_val = &d["scenes"];
      if (scenes_val->IsArray()) {
        for (rapidjson::SizeType i = 0; i < scenes_val->Size();i++) {
          data.push_back(new SceneData(scenes_val[i]));
        }
      }
    }
  }
  else { obj_logging->error("JSON Message detected that is not an object"); }
}

std::string Scene::to_json() {
  obj_logging->info("To JSON message Called on Scene");
	//Initialize the string buffer and writer
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);

  //Start writing the object
	//Syntax taken directly from
	//simplewriter.cpp in rapidjson examples

	writer.StartObject();

  //Add the message type
	writer.Key("msg_type");
	writer.Uint(msg_type);
  //Add the error code
  writer.Key("err_code");
  writer.Uint(err_code);
  //Add the error message
  if (!err_msg.empty()) {
		writer.Key("err_msg");
		writer.String( err_msg.c_str(), (rapidjson::SizeType)err_msg.length() );
	}
  //Add the Transaction ID
  if (!(transaction_id.empty())) {
		writer.Key("transaction_id");
		writer.String( transaction_id.c_str(), (rapidjson::SizeType)transaction_id.length() );
	}
  //Determine how many scenes we want to return in the message
  int num_scenes = static_cast<int>(data.size());
  if ( (num_records != -1) && (num_records < num_scenes) ) {
    num_scenes = num_records;
  }
  //Add the scene list
  writer.Key("scenes");
  writer.StartArray();
  //Iterate through the data list and process each scene
  for (int a = 0; a < num_scenes; a++) {
    writer.StartObject();

    //Add the key
    if (!(data[a]->get_key().empty())) {
      writer.Key("key");
      writer.String( data[a]->get_key().c_str(), (rapidjson::SizeType)data[a]->get_key().length() );
    }
    //Add the name
    if (!(data[a]->get_name().empty())) {
      writer.Key("name");
      writer.String( data[a]->get_name().c_str(), (rapidjson::SizeType)data[a]->get_name().length() );
    }
    //Add the latitude
    if (data[a]->get_latitude() > -9999.0) {
      writer.Key("latitude");
      writer.Double(data[a]->get_latitude());
    }
    //Add the longitude
    if (data[a]->get_longitude() > -9999.0) {
      writer.Key("longitude");
      writer.Double(data[a]->get_longitude());
    }
    //Add the distance
    if (data[a]->get_distance() > -9999.0) {
      writer.Key("distance");
      writer.Double(data[a]->get_distance());
    }

    //Add the scene transform
    if (data[a]->has_transform()) {
      writer.Key("transform");
      writer.StartObject();
      if (data[a]->get_scene_transform()->has_translation()) {
        //Add the translation
        writer.Key("translation");
        writer.StartArray();
        writer.Double(data[a]->get_scene_transform()->translation(0));
        writer.Double(data[a]->get_scene_transform()->translation(1));
        writer.Double(data[a]->get_scene_transform()->translation(2));
        writer.EndArray();
      }
      if (data[a]->get_scene_transform()->has_rotation()) {
        //Add the rotation
        writer.Key("rotation");
        writer.StartArray();
        writer.Double(data[a]->get_scene_transform()->rotation(0));
        writer.Double(data[a]->get_scene_transform()->rotation(1));
        writer.Double(data[a]->get_scene_transform()->rotation(2));
        writer.EndArray();
      }
      writer.EndObject();
    }

    //Add the device list
    writer.Key("devices");
    writer.StartArray();

    //Iterate through the device list for the scene
    for (int b = 0; b < data[a]->num_devices(); b++) {
      writer.StartObject();
      //Add the key
      if (!(data[a]->get_device(b)->get_key().empty())) {
        writer.Key("key");
        writer.String( data[a]->get_device(b)->get_key().c_str(), (rapidjson::SizeType)data[a]->get_device(b)->get_key().length() );
      }
      //Add the device transform
      if (data[a]->get_device(b)->has_transform()) {
        writer.Key("transform");
        writer.StartObject();
        if (data[a]->get_device(b)->get_transform()->has_translation()) {
          //Add the translation
          writer.Key("translation");
          writer.StartArray();
          writer.Double(data[a]->get_device(b)->get_transform()->translation(0));
          writer.Double(data[a]->get_device(b)->get_transform()->translation(1));
          writer.Double(data[a]->get_device(b)->get_transform()->translation(2));
          writer.EndArray();
        }
        if (data[a]->get_device(b)->get_transform()->has_rotation()) {
          //Add the rotation
          writer.Key("rotation");
          writer.StartArray();
          writer.Double(data[a]->get_device(b)->get_transform()->rotation(0));
          writer.Double(data[a]->get_device(b)->get_transform()->rotation(1));
          writer.Double(data[a]->get_device(b)->get_transform()->rotation(2));
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

  //The Stringbuffer now contains a json message
	//of the object
  ret_val = s.GetString();
  ret_string.assign(ret_val);
	return ret_string;
}
