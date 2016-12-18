#include "scene.h"

//Create a new user device object from a protocol buffer user device object
UserDevice::UserDevice(protoScene::SceneList_UserDevice ud_data) {
  std::string new_key = "";
  Eigen::Matrix4d new_transform=Eigen::Matrix4d::Identity(4, 4);
  if (ud_data.has_key()) {
    new_key = ud_data.key();
  }
  if (ud_data.has_transform()) {
    protoScene::SceneList_Matrix4 trn = ud_data.transform();
    for (int i=0; i < trn.col_size(); i++) {
      protoScene::SceneList_Vertex4 c = trn.col(i);
      new_transform(0, i) = c.w();
      new_transform(1, i) = c.x();
      new_transform(2, i) = c.y();
      new_transform(3, i) = c.z();
    }
    obj_logging->debug("Transform Matrix Parsed");
		transform_matrix=new_transform;
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
  Eigen::Matrix4d new_transform=Eigen::Matrix4d::Identity(4, 4);
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
  if (scn_data.has_name()) {
		new_name = scn_data.name();
	}
  if (scn_data.has_transform()) {
    protoScene::SceneList_Matrix4 trn = scn_data.transform();
    for (int i=0; i < trn.col_size(); i++) {
      protoScene::SceneList_Vertex4 c = trn.col(i);
      new_transform(0, i) = c.w();
      new_transform(1, i) = c.x();
      new_transform(2, i) = c.y();
      new_transform(3, i) = c.z();
    }
    obj_logging->debug("Transform Matrix Parsed");
		transform_matrix=new_transform;
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
}

//Constructor
Scene::Scene() {
  msg_type=-1;
  err_code=100;
  err_msg = "";
  transaction_id = "";
}

//Constructor accepting Protocol Buffer
Scene::Scene(protoScene::SceneList buffer) {

  //Fields to store the new variable values
  int new_message_type = -1;
  std::string new_err_msg = "";
  int new_err_code = 100;
  std::string new_transaction_id = "";

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
  for (int a = 0; a < buffer.scenes_size(); a++) {
    SceneData *sc_data = new SceneData (buffer.scenes(a));
    data.push_back(sc_data);
  }

  //Assign the new values
  msg_type = new_message_type;
  err_code = new_err_code;
  err_msg = new_err_msg;
  transaction_id = new_transaction_id;
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

  //Iterate through the data list and process each scene
  for (unsigned int a = 0; a < data.size(); a++) {
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
  		protoScene::SceneList_Matrix4 *trn = scn->mutable_transform();
  		for (int i = 0; i < 4; i++) {
  			protoScene::SceneList_Vertex4* c1 = trn->add_col();
  			c1->set_w(data[a]->get_transform(0, i));
  			c1->set_x(data[a]->get_transform(1, i));
  			c1->set_y(data[a]->get_transform(2, i));
  			c1->set_z(data[a]->get_transform(3, i));
  		}
  	}

    //Iterate through the device list for the scene and write those
    for (int b = 0; b < data[a]->num_devices(); b++) {
      protoScene::SceneList_UserDevice *ud = scn->add_devices();
      std::string key = data[a]->get_device(b)->get_key();
      if ( !(key.empty()) ) {
    		ud->set_key(key);
    	}
      if (data[a]->get_device(b)->has_transform()) {
        protoScene::SceneList_Matrix4 *tran = ud->mutable_transform();
    		for (int i = 0; i < 4; i++) {
    			protoScene::SceneList_Vertex4* c = tran->add_col();
    			c->set_w(data[a]->get_device(b)->get_transform(0, i));
    			c->set_x(data[a]->get_device(b)->get_transform(1, i));
    			c->set_y(data[a]->get_device(b)->get_transform(2, i));
    			c->set_z(data[a]->get_device(b)->get_transform(3, i));
    		}
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
