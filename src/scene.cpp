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
  tran[0] = 0.0;
  tran[1] = 0.0;
  tran[2] = 0.0;
  rot[0] = 0.0;
  rot[1] = 0.0;
  rot[2] = 0.0;
}

Transform::Transform(protoScene::SceneList_Transformation data) {
  tran_flag=false;
  rot_flag=false;
  tran[0] = 0.0;
  tran[1] = 0.0;
  tran[2] = 0.0;
  rot[0] = 0.0;
  rot[1] = 0.0;
  rot[2] = 0.0;
  if (data.has_translation()) {
    protoScene::SceneList_Vertex3 ptrans = data.translation();
    tran[0] = ptrans.x();
    tran[1] = ptrans.y();
    tran[2] = ptrans.z();
    tran_flag=true;
  }
  if (data.has_rotation()) {
    protoScene::SceneList_Vertex3 prot = data.rotation();
    rot[0] = prot.x();
    rot[1] = prot.y();
    rot[2] = prot.z();
    rot_flag=true;
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
  std::string new_key = "";
  if (ud_data.has_key()) {
    new_key = ud_data.key();
  }
  if (ud_data.has_transform()) {
    protoScene::SceneList_Transformation trn = ud_data.transform();
    trans = new Transform (trn);
		trns_flag=true;
  }
  key=new_key;
}

//Create a new scene data object from a protocol buffer scene object
SceneData::SceneData(protoScene::SceneList_Scene scn_data) {

  //Clear the vector data stored in the Scene
  devices.clear();

  //New variables
  std::string new_key="";
  std::string new_name="";
  double new_latitude=0.0;
  double new_longitude=0.0;
  double new_distance=0.0;
  trns_flag=false;

  //Perform the translation
  if (scn_data.has_key()) {
    new_key = scn_data.key();
  }
  if (scn_data.has_latitude()) {
		new_latitude = scn_data.latitude();
	}
  if (scn_data.has_longitude()) {
		new_longitude = scn_data.longitude();
	}
  if (scn_data.has_distance()) {
		new_distance = scn_data.distance();
	}
  if (scn_data.has_name()) {
		new_name = scn_data.name();
	}
  if (scn_data.has_transform()) {
    protoScene::SceneList_Transformation trn = scn_data.transform();
    scene_transform = new Transform (trn);
		trns_flag=true;
  }
  if (scn_data.devices_size() > 0) {
    for (int k=0; k< scn_data.devices_size(); k++) {
      UserDevice *ud = new UserDevice (scn_data.devices(k));
      devices.push_back(ud);
    }
  }

  //Assign the variables
  key = new_key;
  name = new_name;
  latitude = new_latitude;
  longitude = new_longitude;
  distance=new_distance;
}

//Constructor
Scene::Scene() {
  msg_type=-1;
  err_code=100;
  err_msg = "";
  transaction_id = "";
  distance = -1.0;
  num_records = 10;
}

//Constructor accepting Protocol Buffer
Scene::Scene(protoScene::SceneList buffer) {

  //Fields to store the new variable values
  int new_message_type = -1;
  std::string new_err_msg = "";
  int new_err_code = 100;
  std::string new_transaction_id = "";
  double new_distance = -1.0;
  int new_num_records = 10;

  //Perform the conversion
  if (buffer.has_message_type()) {
		new_message_type = buffer.message_type();
	}
  if (buffer.has_transaction_id()) {
    new_transaction_id = buffer.transaction_id();
  }
  if (buffer.has_err_msg()) {
    new_err_msg = buffer.err_msg();
  }
  if (buffer.has_err_code()) {
    new_err_code = buffer.err_code();
  }
  if (buffer.has_distance()) {
    new_distance = buffer.distance();
  }
  if (buffer.has_num_records()) {
    new_num_records = buffer.num_records();
  }
  for (int a = 0; a < buffer.scenes_size(); a++) {
    SceneData *sc_data = new SceneData (buffer.scenes(a));
    data.push_back(sc_data);
  }

  //Assign the new values
  msg_type = new_message_type;
  err_code = new_err_code;
  err_msg = new_err_msg;
  transaction_id = new_transaction_id;
  distance = new_distance;
  num_records = new_num_records;
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
  new_proto->set_distance(distance);

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
  std::string wstr;
  new_proto->SerializeToString(&wstr);
	obj_logging->debug("Protocol Buffer Serialized to String");
	obj_logging->debug(wstr);
  if (new_proto) {
	   delete new_proto;
  }
	return wstr;
}
