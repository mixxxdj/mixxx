#pragma once
#include <libremidi/backends/coremidi/error_domain.hpp>
#include <libremidi/detail/memory.hpp>
#include <libremidi/detail/midi_api.hpp>
#include <libremidi/error_handler.hpp>
#include <libremidi/input_configuration.hpp>

#include <CoreMIDI/CoreMIDI.h>
#include <CoreServices/CoreServices.h>

#include <cmath>

#include <bit>

#if TARGET_OS_IPHONE
  #include <CoreAudio/CoreAudioTypes.h>
  #include <mach/mach_time.h>
  #define LIBREMIDI_AUDIO_GET_CURRENT_HOST_TIME mach_absolute_time
#else
  #include <CoreAudio/HostTime.h>
  #define LIBREMIDI_AUDIO_GET_CURRENT_HOST_TIME AudioGetCurrentHostTime
#endif

NAMESPACE_LIBREMIDI
{
using CFString_handle = unique_handle<const __CFString, CFRelease>;
using CFStringMutable_handle = unique_handle<__CFString, CFRelease>;

LIBREMIDI_STATIC std::string get_string_property(MIDIObjectRef object, CFStringRef property) noexcept
{
  CFStringRef res;
  if (MIDIObjectGetStringProperty(object, property, &res) || !res)
    return {};

  char name[256];
  CFStringGetCString(res, name, sizeof(name), kCFStringEncodingUTF8);
  CFRelease(res);
  return name;
}

LIBREMIDI_STATIC int32_t get_int_property(MIDIObjectRef object, CFStringRef property) noexcept
{
  SInt32 res;
  MIDIObjectGetIntegerProperty(object, property, &res);
  return res;
}

LIBREMIDI_STATIC CFString_handle toCFString(std::string_view str) noexcept
{
  return CFString_handle{CFStringCreateWithCString(nullptr, str.data(), kCFStringEncodingASCII)};
}

#if TARGET_OS_IPHONE
inline uint64_t AudioConvertHostTimeToNanos(uint64_t hostTime)
{
  static const struct mach_timebase_info timebase = [] {
    struct mach_timebase_info theTimeBaseInfo;
    mach_timebase_info(&theTimeBaseInfo);
    return theTimeBaseInfo;
  }();
  const auto numer = timebase.numer;
  const auto denom = timebase.denom;

  __uint128_t res = hostTime;
  if (numer != denom)
  {
    res *= numer;
    res /= denom;
  }
  return static_cast<uint64_t>(res);
}
#endif

LIBREMIDI_STATIC auto get_cfstring_property(MIDIObjectRef prop, CFStringRef name)
{
  CFStringRef str = nullptr;
  MIDIObjectGetStringProperty(prop, name, &str);
  return CFString_handle{str};
}

// This function was submitted by Douglas Casey Tucker and apparently
// derived largely from PortMidi.
inline CFStringRef EndpointName(MIDIEndpointRef endpoint, bool isExternal)
{
  CFMutableStringRef result = CFStringCreateMutable(nullptr, 0);

  // Begin with the endpoint's name.
  if (auto endpoint_name = get_cfstring_property(endpoint, kMIDIPropertyName))
  {
    CFStringAppend(result, endpoint_name.get());
  }

  // some MIDI devices have a leading space in endpoint name. trim
  CFStringTrim(result, CFSTR(" "));

  MIDIEntityRef entity = 0;
  MIDIDeviceRef device = 0;
  MIDIEndpointGetEntity(endpoint, &entity);
  if (entity == 0)
    goto finish;

  if (CFStringGetLength(result) == 0)
  {
    // endpoint name has zero length -- try the entity
    if (auto entity_name = get_cfstring_property(entity, kMIDIPropertyName))
    {
      CFStringAppend(result, entity_name.get());
    }
  }

  // now consider the device's name
  MIDIEntityGetDevice(entity, &device);
  if (device == 0)
    goto finish;

  if (auto dev_name = get_cfstring_property(device, kMIDIPropertyName))
  {
    const auto dev_strlen = CFStringGetLength(dev_name.get());

    // if an external device has only one entity, throw away
    // the endpoint name and just use the device name
    if (CFStringGetLength(result) == 0
        || (isExternal && MIDIDeviceGetNumberOfEntities(device) < 2))
    {
      CFStringAppend(result, dev_name.get());
      goto finish;
    }

    // does the entity name already start with the device name?
    // (some drivers do this though they shouldn't)
    // if so, do not prepend
    if (CFStringCompareWithOptions(result, dev_name.get(), CFRangeMake(0, dev_strlen), 0)
        != kCFCompareEqualTo)
    {
      // prepend the device name to the entity name
      if (CFStringGetLength(result) > 0)
        CFStringInsert(result, 0, CFSTR(" "));
      CFStringInsert(result, 0, dev_name.get());
    }
  }

finish:
  if (CFStringGetLength(result) == 0)
    return CFSTR("No name");
  else
    return result;
}

// This function was submitted by Douglas Casey Tucker and apparently
// derived largely from PortMidi.
inline CFStringRef ConnectedEndpointName(MIDIEndpointRef endpoint)
{
  CFMutableStringRef result = CFStringCreateMutable(nullptr, 0);
  CFStringRef str{};

  // Does the endpoint have connections?
  CFDataRef connections = nullptr;
  std::size_t nConnected = 0;
  bool anyStrings = false;
  MIDIObjectGetDataProperty(endpoint, kMIDIPropertyConnectionUniqueID, &connections);
  if (connections != nullptr)
  {
    // It has connections, follow them
    // Concatenate the names of all connected devices
    nConnected = CFDataGetLength(connections) / sizeof(MIDIUniqueID);
    if (nConnected)
    {
      const SInt32* pid = (const SInt32*)(CFDataGetBytePtr(connections));
      for (std::size_t i = 0; i < nConnected; ++i, ++pid)
      {
        MIDIUniqueID id = CFSwapInt32BigToHost(*pid);
        MIDIObjectRef connObject;
        MIDIObjectType connObjectType;
        auto err = MIDIObjectFindByUniqueID(id, &connObject, &connObjectType);
        if (err == noErr)
        {
          if (connObjectType == kMIDIObjectType_ExternalSource
              || connObjectType == kMIDIObjectType_ExternalDestination)
          {
            // Connected to an external device's endpoint (10.3 and later).
            str = EndpointName((MIDIEndpointRef)(connObject), true);
          }
          else
          {
            // Connected to an external device (10.2) (or something else,
            // catch-
            str = nullptr;
            MIDIObjectGetStringProperty(connObject, kMIDIPropertyName, &str);
          }
          if (str != nullptr)
          {
            if (anyStrings)
              CFStringAppend(result, CFSTR(", "));
            else
              anyStrings = true;
            CFStringAppend(result, str);
            CFRelease(str);
          }
        }
      }
    }
    CFRelease(connections);
  }
  if (anyStrings)
    return result;

  CFRelease(result);

  // Here, either the endpoint had no connections, or we failed to obtain names
  return EndpointName(endpoint, false);
}

inline MIDIObjectRef
locate_object(auto& self, const port_information& info, MIDIObjectType requested_type)
{
  auto uid = std::bit_cast<std::int32_t>((uint32_t)info.port);
  MIDIObjectRef object{};
  MIDIObjectType type{};
  auto ret = MIDIObjectFindByUniqueID(uid, &object, &type);
  if (ret != noErr)
  {
    self.libremidi_handle_error(self.configuration, "cannot find port: " + info.port_name);
    return 0;
  }

  if (type != requested_type || object == 0)
  {
    self.libremidi_handle_error(
        self.configuration, "invalid object: " + info.port_name + " : " + std::to_string(object));
    return 0;
  }

  return object;
}

// A structure to hold variables related to the CoreMIDI API
// implementation.
struct coremidi_data
{
  MIDIClientRef client{};
  MIDIPortRef port{};
  MIDIEndpointRef endpoint = 0;

  [[nodiscard]] OSStatus init_client(auto& configuration)
  {
    if (configuration.context)
    {
      client = *configuration.context;
      return noErr;
    }
    else
    {
      // Set up our client.
      return MIDIClientCreate(
          toCFString(configuration.client_name).get(), nullptr, nullptr, &client);
    }
  }

  void close_client(auto& self)
  {
    self.client_open_ = std::errc::not_connected;
    if (!self.configuration.context)
      MIDIClientDispose(self.client);
  }

  static uint64_t time_in_nanos(MIDITimeStamp tp) noexcept
  {
    if (tp == 0)
    { // this happens when receiving asynchronous sysex messages
      return clock_gettime_nsec_np(CLOCK_UPTIME_RAW);
    }
    else
    {
      return AudioConvertHostTimeToNanos(tp);
    }
  }

  static void set_timestamp(auto& self, MIDITimeStamp packet, timestamp& msg) noexcept
  {
    // packet.timeStamp is in mach_absolute_time units
    // We want a timestamp in nanoseconds

    switch (self.configuration.timestamps)
    {
      case timestamp_mode::NoTimestamp:
        msg = 0;
        return;
      case timestamp_mode::Relative: {
        if (self.firstMessage)
        {
          self.firstMessage = false;
          msg = 0;
          return;
        }
        else
        {
          if constexpr (requires { self.continueSysex; })
          {
            if (self.continueSysex)
              return;
          }

          auto time = time_in_nanos(packet);
          time -= self.last_time;
          msg = time;
        }
        break;
      }
      case timestamp_mode::Absolute:
      case timestamp_mode::SystemMonotonic:
        if constexpr (requires { self.continueSysex; })
        {
          if (self.continueSysex)
            return;
        }
        msg = time_in_nanos(packet);
        break;

      case timestamp_mode::Custom: {
        if constexpr (requires { self.continueSysex; })
        {
          if (self.continueSysex)
            return;
        }
        msg = self.configuration.get_timestamp(time_in_nanos(packet));
        break;
      }
    }
  }

  stdx::error close_port()
  {
    if (this->endpoint)
    {
      MIDIEndpointDispose(this->endpoint);
      this->endpoint = 0;
    }

    if (this->port)
    {
      MIDIPortDispose(this->port);
      this->port = 0;
    }

    return stdx::error{};
  }
};

}
