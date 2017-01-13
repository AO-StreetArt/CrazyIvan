//Message accessor for SceneList Protocol Buffer object access
//Message creator for SceneList Protcol Buffer object

#include <string>
#include <vector>
#include <algorithm>

#include <Eigen/Dense>

#include "ivan_log.h"
#include "ivan_utils.h"
#include "redis_locking.h"
#include "configuration_manager.h"
#include "Scene.pb.h"

#ifndef SCENE
#define SCENE

//Transform
//Stores a translation, local rotation, and global rotation
class Transform {
std::vector<double> tran;
std::vector<double> rot;
bool tran_flag;
bool rot_flag;
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
};

//User Device
//Stores a transform (relative to current scene) and key
class UserDevice {
Transform *trans = NULL;
std::string key;
bool trns_flag;
public:
  UserDevice(protoScene::SceneList_UserDevice scn_data);
  UserDevice() {trns_flag=false;}
  UserDevice(Transform *transform) {trans = transform;trns_flag=true;}
  UserDevice(std::string new_key) {key = new_key;trns_flag=false;}
  UserDevice(std::string new_key, Transform *transform) {key = new_key;trans = transform;trns_flag=true;}
  UserDevice(const UserDevice &ud);
  ~UserDevice() {if (trns_flag) {delete trans;}}
  void set_transform(Transform *transform) {trans = transform;trns_flag=true;}
  Transform* get_transform() const {return trans;}
  double get_translation(int index) const {return trans->translation(index);}
  double get_rotation(int index) const {return trans->rotation(index);}
  void set_translation(int index, double amt) {trans->translate(index, amt);trns_flag=true;}
  void set_rotation(int index, double amt) {trans->rotate(index, amt);trns_flag=true;}
  bool has_transform() const {if ((!trans) || (!trns_flag)) {return false;} else {return true;}}
  void set_key(std::string new_key) {key = new_key;}
  std::string get_key() const {return key;}
};

//Stores the data for a single scene
class SceneData {
std::string key;
std::string name;
double latitude;
double longitude;
double distance;
bool trns_flag;
std::vector<UserDevice> devices;
Transform* scene_transform;
public:
  SceneData() {key = "";name = "";latitude=0.0;longitude=0.0;distance=0.0;trns_flag=false;}
  SceneData(protoScene::SceneList_Scene scn_data);
  SceneData(const SceneData& sd);
  ~SceneData() {if (trns_flag) {delete scene_transform;} }
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
  void add_device(UserDevice d) {devices.push_back(d);}
  int num_devices() const {return devices.size();}
  UserDevice get_device(int index) const {return devices[index];}
  std::vector<UserDevice> get_devices() const {return devices;}

  //Transform
  Transform* get_scene_transform() const {return scene_transform;}
  bool has_transform() const {return trns_flag;}
  void set_transform(Transform *trns) {scene_transform=trns;}
};

//A single scene message, may include data for multiple scene objects
class Scene {
int msg_type;
std::string err_msg;
int err_code;
std::string transaction_id;
std::vector<SceneData> data;
double distance;
int num_records;
public:
  //Constructor
  Scene();
  //Constructor accepting Protocol Buffer serialized string
  //Here we parse the string and populate the scene object with the information
  Scene(protoScene::SceneList buffer);
  //Destructor
  ~Scene() {}
  //Convert the scene information into a Protocol Buffer serialized message
  std::string to_protobuf();
  //Setters
  void set_msg_type(int new_msg_type) {msg_type=new_msg_type;}
  void set_err_msg(std::string new_err) {err_msg=new_err;}
  void set_transaction_id(std::string new_tran_id) {transaction_id=new_tran_id;}
  void set_err_code(int new_code) {err_code=new_code;}
  void add_scene(SceneData scn) {data.push_back(scn);}
  void set_distance(double new_dist) {distance = new_dist;}
  void set_num_records(int new_num) {num_records = new_num;}
  //Getters
  int get_msg_type() {return msg_type;}
  std::string get_err() {return err_msg;}
  std::string get_transaction_id() {return transaction_id;}
  int get_err_code() {return err_code;}
  SceneData get_scene(int i) {return data[i];}
  int num_scenes() {return data.size();}
  double get_distance() {return distance;}
  int get_num_records() {return num_records;}

};

#endif
