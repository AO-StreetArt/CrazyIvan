#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <vector>
#include <utility>
#include <exception>
#include <thread>

#include "aossl/profile/include/tiered_app_profile.h"

#include "Poco/ThreadPool.h"
#include "Poco/Runnable.h"
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
const int EVENT_LENGTH = 550;

// Global atomic booleans for shutting down
std::atomic<bool> is_app_running(false);
std::atomic<bool> is_sender_running(false);

// Send UDP updates to client devices
// also responsible for cleaning up the event memory
// TO-DO: Socket Pool Implementation
class EventSender : public Poco::Runnable {
  DeviceCache *cache = NULL;
  char *event = NULL;
  boost::asio::io_service *io_service = NULL;
  bool encrypted = false;
  bool decrypted = false;
  std::string encrypt_key;
  std::string encrypt_iv;
  std::string decrypt_key;
  std::string decrypt_iv;
  Poco::Logger& logger;
public:
  EventSender(DeviceCache *c, char *evt, boost::asio::io_service &ios) : logger(Poco::Logger::get("Event")) {
    cache = c;
    event = evt;
    io_service = &ios;
  }
  EventSender(DeviceCache *c, char *evt, boost::asio::io_service &ios, std::string& ekey, std::string& eiv, std::string& dkey, std::string& deciv) : logger(Poco::Logger::get("Event"))  {
    cache = c;
    event = evt;
    io_service = &ios;
    encrypted = true;
    decrypted = true;
    encrypt_key.assign(ekey);
    encrypt_iv.assign(eiv);
    decrypt_key.assign(dkey);
    decrypt_iv.assign(deciv);
  }
  virtual void run() {
    logger.debug("Sending Object Updates");
    Poco::Crypto::CipherFactory& factory = Poco::Crypto::CipherFactory::defaultFactory();
    // Creates a 256-bit AES cipher (one for encryption, one for decryption)
    std::vector<unsigned char> encrypt_key_vect(encrypt_key.begin(), encrypt_key.end());
    std::vector<unsigned char> encrypt_iv_vect(encrypt_iv.begin(), encrypt_iv.end());
    std::vector<unsigned char> decrypt_key_vect(decrypt_key.begin(), decrypt_key.end());
    std::vector<unsigned char> decrypt_iv_vect(decrypt_iv.begin(), decrypt_iv.end());
    Poco::Crypto::Cipher* eCipher = factory.createCipher(Poco::Crypto::CipherKey("aes-256-cbc", encrypt_key_vect, encrypt_iv_vect));
    Poco::Crypto::Cipher* dCipher = factory.createCipher(Poco::Crypto::CipherKey("aes-256-cbc", decrypt_key_vect, decrypt_iv_vect));
    // Find the Scene ID
    std::string event_string(event);
    logger.debug(event_string);
    std::vector<std::string> event_items;
    if (decrypted) {
      std::string decrypted = dCipher->decryptString(event_string, Poco::Crypto::Cipher::ENC_BASE64);
      split_from_index(decrypted, event_items, '\n', 0);
    } else {
      split_from_index(event_string, event_items, '\n', 0);
    }
    for (auto elt : event_items) {
      logger.debug(elt);
    }
    // Build a UDP Socket to send our messages
    boost::asio::ip::udp::socket socket(*io_service);
    socket.open(boost::asio::ip::udp::v4());
    // Scene ID is now in entry 0, event json is in entry 1
    // load the scene out of the cache
    std::vector<std::pair<std::string, int>> found_devices = cache->get_devices(event_items[0]);
    logger.debug("Found Total Number of Devices %s", found_devices.size());
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
    delete eCipher;
    delete dCipher;
  }
};

// Central Event Stream Process, which listens on a UDP port
void event_stream(DeviceCache *cache, AOSSL::TieredApplicationProfile *config) {
  Poco::Logger& logger = Poco::Logger::get("Event");
  logger.information("Starting Event Stream");
  is_sender_running = true;
  std::vector<EventSender*> evt_senders {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
  try {
    // Get the udp port out of the configuration profile
    AOSSL::StringBuffer udp_port;
    config->get_opt(std::string("udp.port"), udp_port);
    int port = std::stoi(udp_port.val);
    bool aes_enabled = false;
    int sender_index = 0;
    // Open the UDP Socket
    boost::asio::io_service io_service;
    boost::asio::ip::udp::socket socket(io_service, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), port));
    // Listen on the UDP Socket
    while (is_app_running.load()) {
      // Load Security config, having it here allows dynamic
      // security updates.
      AOSSL::StringBuffer aes_enabled_buffer;
      AOSSL::StringBuffer aesin_key_buffer;
      AOSSL::StringBuffer aesin_iv_buffer;
      AOSSL::StringBuffer aesout_key_buffer;
      AOSSL::StringBuffer aesout_iv_buffer;
      config->get_opt(std::string("event.security.aes.enabled"), aes_enabled_buffer);
      config->get_opt(config->get_cluster_name() + \
          std::string(".event.security.out.aes.key"), aesout_key_buffer);
      config->get_opt(config->get_cluster_name() + \
          std::string(".event.security.out.aes.iv"), aesout_iv_buffer);
      config->get_opt(config->get_cluster_name() + \
          std::string(".event.security.in.aes.key"), aesin_key_buffer);
      config->get_opt(config->get_cluster_name() + \
          std::string(".event.security.in.aes.iv"), aesin_iv_buffer);
      if (aes_enabled_buffer.val == "true") aes_enabled = true;
      boost::asio::ip::udp::endpoint remote_endpoint;
      boost::system::error_code error;

      // First, we call recieve_from on the socket with a null buffer,
      // which returns when a message is on the recieve queue with the number
      // of bytes in the message
      socket.receive_from(boost::asio::null_buffers(), remote_endpoint, 0, error);
      int available = socket.available();

      // Build a buffer the size of the available data
      char recv_buf[available];
      boost::asio::mutable_buffers_1 bbuffer = boost::asio::buffer(recv_buf, available);

      // Copy the data from the input queue to the buffer
      int bytes_transferred = socket.receive_from(bbuffer, remote_endpoint, 0, error);
      char* event_data_ptr = boost::asio::buffer_cast<char*>(bbuffer);
      if (!(error && error != boost::asio::error::message_size && bytes_transferred > 0) \
          && is_app_running.load()) {
        logger.debug("Recieved UDP Update");

        // Copy the message buffer into dynamic memory
        char *event_msg = new char[available+1]();
        memcpy(event_msg, event_data_ptr, bytes_transferred);
        logger.debug(event_msg);

        // Clear out any left-over event sender
        if (evt_senders[sender_index]) delete evt_senders[sender_index];
        // Build the new event sender
        if (aes_enabled) {
          evt_senders[sender_index] = new EventSender(cache, event_msg, io_service, aesout_key_buffer.val, aesout_iv_buffer.val, aesin_key_buffer.val, aesin_iv_buffer.val);
        } else {
          evt_senders[sender_index] = new EventSender(cache, event_msg, io_service);
        }

        // Fire off another thread to actually send UDP messages
        if (sender_index == 12) {
            // If we have used up all the space in our array of senders,
            // then we should wait for other threads to complete before
            // pulling the next message.
            Poco::ThreadPool::defaultPool().joinAll();
            sender_index = 0;
        }
        try {
          Poco::ThreadPool::defaultPool().start(*(evt_senders[sender_index]));
          sender_index = sender_index + 1;
        } catch (Poco::NoThreadAvailableException& e) {
          // If no more threads are available, then we execute on the main
          // thread, and wait for other threads to complete before
          // pulling the next message.
          evt_senders[sender_index]->run();
          Poco::ThreadPool::defaultPool().joinAll();
          sender_index = 0;
        }
      }
    }
  } catch (std::exception& e) {
    logger.error(e.what());
  }
  for (EventSender *sender : evt_senders) {
    if (sender) delete sender;
  }
  is_sender_running = false;
}

#endif  // SRC_APP_INCLUDE_EVENT_STREAM_PROCESS_H_
