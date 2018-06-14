#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <vector>
#include <utility>
#include <exception>
#include <thread>

#include "aossl/profile/include/tiered_app_profile.h"

#include "Poco/ThreadPool.h"
#include "Poco/RunnableAdapter.h"
#include "Poco/Crypto/Cipher.h"
#include "Poco/Crypto/CipherFactory.h"
#include "Poco/Crypto/CipherKey.h"



#include "boost/asio.hpp"

#include "ivan_utils.h"

#ifndef SRC_APP_INCLUDE_EVENT_STREAM_PROCESS_H_
#define SRC_APP_INCLUDE_EVENT_STREAM_PROCESS_H_

// 24 bit salt value
// 26 bit scene key
// 1 bit for new line delimiter
// 179 bits for object json
// 1 bit for final new line
const int EVENT_LENGTH = 231;

// Send UDP updates to client devices
// also responsible for cleaning up the event memory
// TO-DO: Socket Pool Implementation
class EventSender {
  DeviceCache *cache = NULL;
  char *event = NULL;
  boost::asio::io_service *io_service = NULL;
  bool encrypted = false;
  bool decrypted = false;
  std::string encrypt_key;
  std::string encrypt_salt;
  std::string decrypt_key;
  std::string decrypt_salt;
  Poco::Logger& logger;
public:
  EventSender(DeviceCache *c, char *evt, boost::asio::io_service &ios) : logger(Poco::Logger::get("Event")) {
    cache = c;
    event = evt;
    io_service = &ios;
  }
  EventSender(DeviceCache *c, char *evt, boost::asio::io_service &ios, std::string& epasswd, std::string& esalt, std::string& dpasswd, std::string& dsalt) : logger(Poco::Logger::get("Event"))  {
    cache = c;
    event = evt;
    io_service = &ios;
    encrypted = true;
    decrypted = true;
    encrypt_key.assign(epasswd);
    encrypt_salt.assign(esalt);
    decrypt_key.assign(dpasswd);
    decrypt_salt.assign(dsalt);
  }
  ~EventSender() {}
  void send_updates() {
    logger.debug("Sending Object Updates");
    Poco::Crypto::CipherFactory& factory = Poco::Crypto::CipherFactory::defaultFactory();
    // Creates a 256-bit AES cipher (one for encryption, one for decryption)
    Poco::Crypto::Cipher* eCipher = factory.createCipher(Poco::Crypto::CipherKey("aes-256", encrypt_key, encrypt_salt));
    Poco::Crypto::Cipher* dCipher = factory.createCipher(Poco::Crypto::CipherKey("aes-256", decrypt_key, decrypt_salt));
    // Find the Scene ID
    std::string event_string(event);
    std::vector<std::string> event_items;
    if (decrypted) {
      std::string decrypted = dCipher->decryptString(event_string, Poco::Crypto::Cipher::ENC_BASE64);
      split(decrypted, event_items, '\n');
    } else {
      split(event_string, event_items, '\n');
    }
    // Build a UDP Socket to send our messages
    boost::asio::ip::udp::socket socket(*io_service);
    socket.open(boost::asio::ip::udp::v4());
    // Scene ID is now in entry 0, event json is in entry 1
    // load the scene out of the cache
    // WARNING: This causes segfaults when the entry isn't in the cache
    //   we need to swap out for std::unordered_map and use find()
    std::vector<std::pair<std::string, int>> found_devices = cache->get_devices(event_items[0]);
    logger.debug("Found Total Number of Devices %d", found_devices.size());
    for (std::pair<std::string, int> device : found_devices) {
      try {
        boost::asio::ip::udp::endpoint remote_endpoint;
        remote_endpoint = boost::asio::ip::udp::endpoint(boost::asio::ip::address::from_string(device.first), device.second);
        boost::system::error_code err;
        if (encrypted) {
          std::string encrypted = eCipher->encryptString(event_items[1], Poco::Crypto::Cipher::ENC_BASE64);
          socket.send_to(boost::asio::buffer(encrypted, encrypted.size()), remote_endpoint, 0, err);
        } else {
          socket.send_to(boost::asio::buffer(event_items[1], event_items[1].size()), remote_endpoint, 0, err);
        }
      } catch (std::exception& e) {
        logger.error(e.what());
      }
    }
    delete[] event;
  }
};

// Central Event Stream Process, which listens on a UDP port
void event_stream(DeviceCache *cache, AOSSL::TieredApplicationProfile *config) {
  Poco::Logger& logger = Poco::Logger::get("Event");
  logger.information("Starting Event Stream");
  try {
    // Get the configuration values out of the configuration profile
    AOSSL::StringBuffer aes_enabled_buffer;
    AOSSL::StringBuffer aesin_key_buffer;
    AOSSL::StringBuffer aesin_salt_buffer;
    AOSSL::StringBuffer aesout_key_buffer;
    AOSSL::StringBuffer aesout_salt_buffer;
    AOSSL::StringBuffer udp_port;
    config->get_opt(std::string("udp.port"), udp_port);
    config->get_opt(std::string("event.security.aes.enabled"), aes_enabled_buffer);
    config->get_opt(std::string("event.security.out.aes.key"), aesout_key_buffer);
    config->get_opt(std::string("event.security.out.aes.salt"), aesout_salt_buffer);
    config->get_opt(std::string("event.security.in.aes.key"), aesin_key_buffer);
    config->get_opt(std::string("event.security.in.aes.salt"), aesin_salt_buffer);
    int port = std::stoi(udp_port.val);
    bool aes_enabled = false;
    if (aes_enabled_buffer.val == "true") aes_enabled = true;
    // Start a std::vector to hold event memory
    std::vector<EventSender*> evt_senders {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
    int sender_index = 0;
    // Open the UDP Socket
    boost::asio::io_service io_service;
    boost::asio::ip::udp::socket socket(io_service, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), port));
    // Listen on the UDP Socket
    while (true) {
      char recv_buf[EVENT_LENGTH];
      boost::asio::ip::udp::endpoint remote_endpoint;
      boost::system::error_code error;
      socket.receive_from(boost::asio::buffer(recv_buf), remote_endpoint, 0, error);
      if (!(error && error != boost::asio::error::message_size)) {
        logger.debug("Recieved UDP Update");
        // Copy the message buffer into dynamic memory
        char *event_msg = new char[EVENT_LENGTH];
        memcpy(event_msg, recv_buf, EVENT_LENGTH);
        // Clear out any left-over event sender
        if (evt_senders[sender_index]) delete evt_senders[sender_index];
        // Build the new event sender
        if (aes_enabled) {
          evt_senders[sender_index] = new EventSender(cache, event_msg, io_service, aesout_key_buffer.val, aesout_salt_buffer.val, aesin_key_buffer.val, aesin_salt_buffer.val);
        } else {
          evt_senders[sender_index] = new EventSender(cache, event_msg, io_service);
        }
        // Fire off another thread to actually send UDP messages
        try {
          Poco::RunnableAdapter<EventSender> runnable(*(evt_senders[sender_index]), &EventSender::send_updates);
          Poco::ThreadPool::defaultPool().start(runnable);
          sender_index = sender_index + 1;
        } catch (Poco::NoThreadAvailableException& e) {
          // If no more threads are available, then execute the updates on the
          // main thread, and wait for other threads to complete before pulling
          // the next message.
          evt_senders[sender_index]->send_updates();
          Poco::ThreadPool::defaultPool().joinAll();
          sender_index = 0;
        }
      }
    }
  } catch (std::exception& e) {
    logger.error(e.what());
  }
}

#endif  // SRC_APP_INCLUDE_EVENT_STREAM_PROCESS_H_
