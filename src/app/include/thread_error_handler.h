#include "Poco/ErrorHandler.h"
#include "Poco/Exception.h"
#include "Poco/Logger.h"

class IvanErrorHandler: public Poco::ErrorHandler {
public:
  void exception(const Poco::Exception& exc) {
    Poco::Logger::get("Event").error(exc.displayText());
  }
  void exception(const std::exception& exc) {
    Poco::Logger::get("Event").error(exc.what());
  }
  void exception() {
    Poco::Logger::get("Event").error("Unknown exception in background thread");
  }
};
