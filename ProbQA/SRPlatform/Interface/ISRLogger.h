#pragma once

#include "../SRPlatform/Interface/SRString.h"

namespace SRPlat {

class SRPLATFORM_API ISRLogger {
public: // Types
  enum class Severity : uint8_t {
    None = 0,
    Info,
    Warning,
    Error,
    Critical
  };
public: // Methods
  virtual ~ISRLogger() { }
  // Returns |false| if an error occurs while logging: a chance for the app to print to screen, show a dialog,
  //   terminate, etc.
  virtual bool Log(const Severity s, const SRString& message) = 0;
};

} // namespace SRPlat

namespace std  {

inline string to_string(const SRPlat::ISRLogger::Severity s) {
  switch (s) {
  case SRPlat::ISRLogger::Severity::None:
    return "None";
  case SRPlat::ISRLogger::Severity::Info:
    return "Info";
  case SRPlat::ISRLogger::Severity::Warning:
    return "Warning";
  case SRPlat::ISRLogger::Severity::Error:
    return "Error";
  case SRPlat::ISRLogger::Severity::Critical:
    return "Critical";
  default:
    return string("Unhandled") + to_string(int32_t(s));
  }
}

} // namespace std
