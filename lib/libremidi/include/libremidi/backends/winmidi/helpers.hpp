#pragma once
// clang-format off
#define NOMINMAX 1
#define WIN32_LEAN_AND_MEAN 1
#include <libremidi/detail/midi_api.hpp>
#include <libremidi/detail/memory.hpp>

#include <cctype>
#include <string>
#include <guiddef.h>
#include <unknwn.h>

#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Devices.Enumeration.h>
#include <winrt/Microsoft.Windows.Devices.Midi2.h>
#include <libremidi/cmidi2.hpp>

#if __has_include(<WindowsMidiServicesAppSdkComExtensions.h>)
#include <WindowsMidiServicesAppSdkComExtensions.h>
  #define LIBREMIDI_WINMIDI_HAS_COM_EXTENSIONS 1

#define LIBREMIDI_DEFINE_GUID_CONSTEXPR(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        LIBREMIDI_STATIC constexpr const type name = {l,w1,w2,{b1,b2,b3,b4,b5,b6,b7,b8}}

NAMESPACE_LIBREMIDI {
  LIBREMIDI_DEFINE_GUID_CONSTEXPR(IID, IID_IMidiEndpointConnectionMessagesReceivedCallback, 0x8087b303, 0x0519, 0x31d1, 0x31, 0xd1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10);
  LIBREMIDI_DEFINE_GUID_CONSTEXPR(IID, IID_IMidiEndpointConnectionRaw,                      0x8087b303, 0x0519, 0x31d1, 0x31, 0xd1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20);
  LIBREMIDI_DEFINE_GUID_CONSTEXPR(IID, IID_IMidiClientInitializer,                          0x8087b303, 0xd551, 0xbce2, 0x1e, 0xad, 0xa2, 0x50, 0x0d, 0x50, 0xc5, 0x80);
  LIBREMIDI_DEFINE_GUID_CONSTEXPR(IID, IID_MidiClientInitializerUuid,                       0xc3263827, 0xc3b0, 0xbdbd, 0x25, 0x00, 0xce, 0x63, 0xa3, 0xf3, 0xf2, 0xc3);
  LIBREMIDI_DEFINE_GUID_CONSTEXPR(IID, IID_MidiSrvTransportUuid,                            0x2ba15e4e, 0x5417, 0x4a66, 0x85, 0xb8, 0x2b, 0x22, 0x60, 0xef, 0xbc, 0x84);
}
#endif

// clang-format on

namespace midi2 = winrt::Microsoft::Windows::Devices::Midi2;
namespace foundation = winrt::Windows::Foundation;

NAMESPACE_LIBREMIDI::winmidi
{
using namespace winrt;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Devices::Enumeration;
using namespace winrt::Windows::Storage::Streams;
using namespace winrt::Microsoft::Windows::Devices::Midi2;

inline bool ichar_equals(char a, char b)
{
  return std::tolower(static_cast<unsigned char>(a))
         == std::tolower(static_cast<unsigned char>(b));
}

inline bool iequals(std::string_view lhs, std::string_view rhs)
{
  return std::ranges::equal(lhs, rhs, ichar_equals);
}

inline std::pair<
    winrt::Microsoft::Windows::Devices::Midi2::MidiEndpointDeviceInformation,
    winrt::Microsoft::Windows::Devices::Midi2::MidiGroupTerminalBlock>
get_port(const std::string& device_name, int group_terminal_block)
{
  auto eps = MidiEndpointDeviceInformation::FindAll();
  for (const auto& ep : eps)
  {
    auto str = to_string(ep.EndpointDeviceId());
    if (str.empty())
      continue;

    if (iequals(str, device_name))
    {
      for (const auto& gp : ep.GetGroupTerminalBlocks())
      {
        if (gp.Number() == group_terminal_block)
        {
          return std::make_pair(ep, gp);
        }
      }
    }
  }
  return {nullptr, nullptr};
}

// From Microsoft.Windows.Devices.Midi2.Initialization.hpp
typedef enum
{
  Platform_x64 = 1,
  //    Platform_Arm64 = 2,
  //    Platform_Arm64EC = 3,
  Platform_Arm64X = 4,
} MidiAppSDKPlatform;

struct IMidiClientInitializer : ::IUnknown
{
  // returns the SDK version info. Supply nullptr for arguments you don't care about
  STDMETHOD(GetInstalledWindowsMidiServicesSdkVersion)(
      MidiAppSDKPlatform* buildPlatform,
      USHORT* versionMajor,
      USHORT* versionMinor,
      USHORT* versionPatch,

      LPWSTR* buildSource,
      LPWSTR* versionName,
      LPWSTR* versionFullString
      ) = 0;

  // demand-starts the service if present
  STDMETHOD(EnsureServiceAvailable)() = 0;
};
struct winmidi_shared_data_instance
{
  IMidiClientInitializer* initializer{nullptr};
  bool ready{false};
  winmidi_shared_data_instance()
  {
    // 1. Check if MIDI Services are available
    {
      ::IUnknown* servicePointer{ nullptr };
      auto hr = CoCreateInstance(
          libremidi::IID_MidiSrvTransportUuid,
          NULL,
          CLSCTX_INPROC_SERVER,
          IID_PPV_ARGS(&servicePointer)
          );

      if (SUCCEEDED(hr))
      {
        if (servicePointer == nullptr)
        {
          ready = false;
          return;
        }

        // Here is the good case:
        servicePointer->Release();
        servicePointer = nullptr;
        ready = true;
      }
      else if (hr == REGDB_E_CLASSNOTREG)
      {
        servicePointer = nullptr;
        ready = false;
        return;
      }
      else
      {
        servicePointer = nullptr;
        ready = false;
        return;
      }
    }

    // 2. Check if MIDI services can be created
    {
      if (SUCCEEDED(CoCreateInstance(
              libremidi::IID_MidiClientInitializerUuid,
              NULL,
              CLSCTX::CLSCTX_INPROC_SERVER | CLSCTX::CLSCTX_FROM_DEFAULT_CONTEXT,
              libremidi::IID_IMidiClientInitializer,
              reinterpret_cast<void**>(&initializer)
              )))
      {
        if (initializer != nullptr)
        {
          ready = true;
        }
        else
        {
          ready = false;
          return;
        }
      }
      else
      {
        ready = false;
        return;
      }
    }

    // 3. Check if MIDI services can be used
    if (SUCCEEDED(initializer->EnsureServiceAvailable()))
    {
      ready = true;
    }
    else
    {
      ready = false;
      initializer->Release();
      initializer = nullptr;
      return;
    }
  }

  ~winmidi_shared_data_instance()
  {
    if (initializer != nullptr)
    {
      initializer->Release();
      initializer = nullptr;
    }
  }
};

struct winmidi_shared_data
{
  std::shared_ptr<winmidi_shared_data_instance> self = libremidi::instance<winmidi_shared_data_instance>();

  winmidi_shared_data()
  {
  }

  ~winmidi_shared_data()
  {
  }
};

}
