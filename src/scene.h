//Message accessor for SceneList Protocol Buffer object access
//Message creator for SceneList Protcol Buffer object

#include <string>
#include <vector>
#include <algorithm>

#include "ivan_log.h"
#include "ivan_utils.h"
#include "configuration_manager.h"
#include "Scene.pb.h"

#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

#ifndef SCENE
#define SCENE

//! A Scene Exception

//! A child class of std::exception
//! which holds error information
struct SceneException: public std::exception
{
  //! An error message passed on initialization
  std::string int_msg;
  const char * int_msg_cstr;

  //! Create a Neo4j Exception, and store the given error message
  SceneException (std::string msg) {int_msg = "Error in Scene: " + msg;int_msg_cstr = int_msg.c_str();}

  SceneException () {}
  ~SceneException() throw () {}
  //! Show the error message in readable format
  const char * what() const throw ()
  {
    return int_msg_cstr;
  }
};

//Transform
//Stores a translation, local rotation, and global rotation
class Transform {
std::vector<double> tran;
std::vector<double> rot;
bool tran_flag;
bool rot_flag;
const rapidjson::Value *translation_val;
const rapidjson::Value *rotation_val;
public:
  Transform();
  Transform(protoScene::SceneList_Transformation data);
  ~Transform() {}
  double translation(int index) const {return tran[index];}
  void translate(int index, double amt) {tran[index] = tran[index] + amt;tran_flag=true;}
  double rotation(int index) const {return rot[index];}
  void rotate(int index, double amt) {rot[index] = rot[index] + amt;rot_flag=true;}
  void add_transform(Transform &t, bool inverted);
  void add_transform(Transform &t) {add_transform(t, false);}
  bool has_translation() const {return tran_flag;}
  bool has_rotation() const {return rot_flag;}
  void invert();
  void clear() {tran.clear();rot.clear();tran_flag=false;rot_flag=false;}
  inline void print() {
    obj_logging->debug("Transformation");
    if (tran_flag) {
      obj_logging->debug("Translation");
      obj_logging->debug(tran[0]);
      obj_logging->debug(tran[1]);
      obj_logging->debug(tran[2]);
    }
    if (rot_flag) {
      obj_logging->debug("Rotation");
      obj_logging->debug(rot[0]);
      obj_logging->debug(rot[1]);
      obj_logging->debug(rot[2]);
    }
  }
};

//User Device
//Stores a transform (relative to current scene) and key
class UserDevice {
Transform *trans = NULL;
std::string key = "";
const char * my_key;
bool trns_flag = false;
const rapidjson::Value *key_val;
const rapidjson::Value *transform_val;
public:
  UserDevice(protoScene::SceneList_UserDevice scn_data);
  UserDevice() {trns_flag=false;}
  UserDevice(Transform *transform) {trans = transform;trns_flag=true;}
  UserDevice(std::string new_key) {key = new_key;trns_flag=false;}
  UserDevice(const char * new_key) {my_key=new_key;key.assign(my_key);}
  UserDevice(std::string new_key, Transform *transform) {key = new_key;trans = transform;trns_flag=true;}
  UserDevice(const UserDevice &ud);
  ~UserDevice() {if (trns_flag) {delete trans;}}
  void set_transform(Transform *transform) {trans = transform;trns_flag=true;}
  Transform* get_transform() const {return trans;}
  double get_translation(int index) const {return trans->translation(index);}
  double get_rotation(int index) const {return trans->rotation(index);}
  void set_translation(int index, double amt) {if (trns_flag) {trans->translate(index, amt);} else {throw SceneException("Attempting to set translation on Device without transform");}}
  void set_rotation(int index, double amt) {if (trns_flag) {trans->rotate(index, amt);} else {throw SceneException("Attempting to set rotation on Device without transform");}}
  bool has_transform() const {if ((!trans) || (!trns_flag)) {return false;} else {return true;}}
  void set_key(std::string new_key) {key = new_key;}
  std::string get_key() const {return key;}
  inline void print() {
    obj_logging->debug("User Device");
    obj_logging->debug(key);
    if (trns_flag) {trans->print();}
  }
};

//Stores the data for a single scene
class SceneData {
std::string key = "";
std::string name = "";
double latitude = 0.0;
double longitude = 0.0;
double distance = 0.0;
bool trns_flag = false;
std::vector<UserDevice*> devices;
Transform* scene_transform;
const rapidjson::Value *key_val;
const rapidjson::Value *scene_val;
const rapidjson::Value *name_val;
const rapidjson::Value *lat_val;
const rapidjson::Value *long_val;
const rapidjson::Value *dist_val;
const rapidjson::Value *transform_val;
const rapidjson::Value *devices_val;
public:
  SceneData() {}
  SceneData(protoScene::SceneList_Scene scn_data);
  SceneData(const SceneData& sd);
  ~SceneData() {if (trns_flag) {delete scene_transform;} for (unsigned int i=0; i<devices.size();i++ ) {if (devices[i]) {delete devices[i];}}}
  //Setters
  void set_key(std::string new_key) {key=new_key;}
  void set_name(std::string new_name) {name=new_name;}
  void set_latitude(double new_lat) {latitude=new_lat;}
  void set_longitude(double new_long) {longitude=new_long;}
  void set_distance(double new_dist) {distance=new_dist;}

  //Getters
  std::string get_key() const {return key;}
  std::string get_name() const {return name;}
  double get_latitude() const {return latitude;}
  double get_longitude() const {return longitude;}
  double get_distance() const {return distance;}

  //List
  void add_device(UserDevice *d) {devices.push_back(d);}
  int num_devices() const {return devices.size();}
  UserDevice* get_device(int index) const {return devices[index];}
  std::vector<UserDevice*> get_devices() const {return devices;}

  //Transform
  Transform* get_scene_transform() const {return scene_transform;}
  bool has_transform() const {return trns_flag;}
  void set_transform(Transform *trns) {scene_transform=trns;trns_flag=true;}

  //Print
  inline void print() {
    obj_logging->debug("Scene Data");
    obj_logging->debug(key);
    obj_logging->debug(name);
    obj_logging->debug(latitude);
    obj_logging->debug(longitude);
    obj_logging->debug(distance);
    for (unsigned int i=0; i<devices.size(); i++) {devices[i]->print();}
    if (trns_flag) {scene_transform->print();}
  }
};

//A single scene message, may include data for multiple scene objects
class Scene {
int msg_type=-1;
std::string err_msg="";
int err_code=100;
std::string transaction_id="";
std::vector<SceneData*> data;
int num_records=10;
std::string protobuf_string="";
const rapidjson::Value *ms_type_val;
const rapidjson::Value *err_code_val;
const rapidjson::Value *err_str_val;
const rapidjson::Value *tran_id_val;
const rapidjson::Value *records_val;
std::string ret_string = "";
const char* ret_val;
public:
  //Constructor
  Scene() {}
  //Constructor accepting Protocol Buffer serialized string
  //Here we parse the string and populate the scene object with the information
  Scene(protoScene::SceneList buffer);
  //Constructor accepting Rapidjson Document as input
  //Parse the string and populate the scene object with the information
  Scene(const rapidjson::Document& d);
  //Destructor
  ~Scene() {for (unsigned int i=0; i<data.size();i++ ) {if (data[i]) {delete data[i];}}}
  //Convert the scene information into a Protocol Buffer serialized message
  std::string to_protobuf();
  //Convert the scene information into a JSON Message
  std::string to_json();
  //Setters
  void set_msg_type(int new_msg_type) {msg_type=new_msg_type;}
  void set_err_msg(std::string new_err) {err_msg=new_err;}
  void set_transaction_id(std::string new_tran_id) {transaction_id=new_tran_id;}
  void set_err_code(int new_code) {err_code=new_code;}
  void add_scene(SceneData *scn) {data.push_back(scn);}
  void set_num_records(int new_num) {num_records = new_num;}
  //Getters
  int get_msg_type() {return msg_type;}
  std::string get_err() {return err_msg;}
  std::string get_transaction_id() {return transaction_id;}
  int get_err_code() {return err_code;}
  SceneData* get_scene(unsigned int i) {if (i<data.size()) {return data[i];} else {throw SceneException("Attempted to access invalid scene data");}}
  int num_scenes() {return data.size();}
  int get_num_records() {return num_records;}
  inline void print() {
    obj_logging->debug("Scene List");
    obj_logging->debug(msg_type);
    obj_logging->debug(err_msg);
    obj_logging->debug(err_code);
    obj_logging->debug(transaction_id);
    obj_logging->debug(num_records);
    for (unsigned int i=0; i<data.size(); i++) {data[i]->print();}
  }
};

#endif
