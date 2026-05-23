#pragma once
#include <libremidi/error.hpp>

#include <CoreMIDI/CoreMIDI.h>

NAMESPACE_LIBREMIDI
{
struct coremidi_error_domain : public stdx::error_domain
{
public:
  constexpr coremidi_error_domain() noexcept
      : error_domain{{0xa32b080ac770514eULL, 0xef59a407f921da43ULL}}
  {
  }

  stdx::string_ref name() const noexcept override { return "coremidi"; }

  bool equivalent(const stdx::error& lhs, const stdx::error& rhs) const noexcept override
  {
    if (lhs.domain() == rhs.domain())
      return error_cast<std::errc>(lhs) == error_cast<std::errc>(rhs);

    return false;
  }

  stdx::string_ref message(const stdx::error& e) const noexcept override
  {
    switch (error_cast<OSStatus>(e))
    {
      case kMIDIInvalidClient:
        return "Invalid Client";
      case kMIDIInvalidPort:
        return "Invalid Port";
      case kMIDIWrongEndpointType:
        return "Wrong EndpointT ype";
      case kMIDINoConnection:
        return "No Connection";
      case kMIDIUnknownEndpoint:
        return "Unknown Endpoint";
      case kMIDIUnknownProperty:
        return "Unknown Property";
      case kMIDIWrongPropertyType:
        return "Wrong Property Type";
      case kMIDINoCurrentSetup:
        return "No Current Setup";
      case kMIDIMessageSendErr:
        return "Message Send Error";
      case kMIDIServerStartErr:
        return "Server Start Error";
      case kMIDISetupFormatErr:
        return "Setup Format Error";
      case kMIDIWrongThread:
        return "Wrong Thread";
      case kMIDIObjectNotFound:
        return "Object Not Found";
      case kMIDIIDNotUnique:
        return "ID Not Unique";
      case kMIDINotPermitted:
        return "Not Permitted";
      case kMIDIUnknownError:
        return "Unknown Error";
    }
    return "Unknown error code";
  }
};

inline stdx::error from_osstatus(OSStatus ret) noexcept
{
  static constexpr coremidi_error_domain domain{};
  return {ret, domain};
}
}
