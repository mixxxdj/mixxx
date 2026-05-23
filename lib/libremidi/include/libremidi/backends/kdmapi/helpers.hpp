#pragma once
// clang-format off
#define NOMINMAX 1
#define WIN32_LEAN_AND_MEAN 1

#include <windows.h>
#include <mmsystem.h>
// clang-format on

#include <libremidi/detail/midi_api.hpp>

#include <atomic>
#include <mutex>

namespace libremidi::kdmapi
{

using IsKDMAPIAvailable_t = BOOL(WINAPI*)();
using InitializeKDMAPIStream_t = BOOL(WINAPI*)();
using TerminateKDMAPIStream_t = BOOL(WINAPI*)();
using ResetKDMAPIStream_t = VOID(WINAPI*)();
using SendDirectData_t = VOID(WINAPI*)(DWORD);
using SendDirectDataNoBuf_t = VOID(WINAPI*)(DWORD);
using SendDirectLongData_t = UINT(WINAPI*)(MIDIHDR*, UINT);
using SendDirectLongDataNoBuf_t = UINT(WINAPI*)(LPSTR, DWORD);
using PrepareLongData_t = UINT(WINAPI*)(MIDIHDR*, UINT);
using UnprepareLongData_t = UINT(WINAPI*)(MIDIHDR*, UINT);
using ReturnKDMAPIVer_t = BOOL(WINAPI*)(LPDWORD, LPDWORD, LPDWORD, LPDWORD);

class kdmapi_loader
{
public:
  static kdmapi_loader& instance()
  {
    static kdmapi_loader loader;
    return loader;
  }

  bool is_available() const noexcept { return m_available; }

  HMODULE handle() const noexcept { return m_handle; }

  // KDMAPI function pointers
  IsKDMAPIAvailable_t IsKDMAPIAvailable{};
  InitializeKDMAPIStream_t InitializeKDMAPIStream{};
  TerminateKDMAPIStream_t TerminateKDMAPIStream{};
  ResetKDMAPIStream_t ResetKDMAPIStream{};
  SendDirectData_t SendDirectData{};
  SendDirectDataNoBuf_t SendDirectDataNoBuf{};
  SendDirectLongData_t SendDirectLongData{};
  SendDirectLongDataNoBuf_t SendDirectLongDataNoBuf{};
  PrepareLongData_t PrepareLongData{};
  UnprepareLongData_t UnprepareLongData{};
  ReturnKDMAPIVer_t ReturnKDMAPIVer{};

private:
  kdmapi_loader()
  {
    // Try to get OmniMIDI if it's already loaded
    m_handle = GetModuleHandleW(L"OmniMIDI");
    if (!m_handle)
    {
      // Try to load it explicitly
      m_handle = LoadLibraryW(L"OmniMIDI.dll");
    }

    if (!m_handle)
    {
      m_available = false;
      return;
    }

#if !defined(_MSC_VER)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-function-type"
#endif

    // Load all function pointers
    IsKDMAPIAvailable
        = reinterpret_cast<IsKDMAPIAvailable_t>(GetProcAddress(m_handle, "IsKDMAPIAvailable"));
    InitializeKDMAPIStream = reinterpret_cast<InitializeKDMAPIStream_t>(
        GetProcAddress(m_handle, "InitializeKDMAPIStream"));
    TerminateKDMAPIStream = reinterpret_cast<TerminateKDMAPIStream_t>(
        GetProcAddress(m_handle, "TerminateKDMAPIStream"));
    ResetKDMAPIStream
        = reinterpret_cast<ResetKDMAPIStream_t>(GetProcAddress(m_handle, "ResetKDMAPIStream"));
    SendDirectData
        = reinterpret_cast<SendDirectData_t>(GetProcAddress(m_handle, "SendDirectData"));
    SendDirectDataNoBuf
        = reinterpret_cast<SendDirectDataNoBuf_t>(GetProcAddress(m_handle, "SendDirectDataNoBuf"));
    SendDirectLongData
        = reinterpret_cast<SendDirectLongData_t>(GetProcAddress(m_handle, "SendDirectLongData"));
    SendDirectLongDataNoBuf = reinterpret_cast<SendDirectLongDataNoBuf_t>(
        GetProcAddress(m_handle, "SendDirectLongDataNoBuf"));
    PrepareLongData
        = reinterpret_cast<PrepareLongData_t>(GetProcAddress(m_handle, "PrepareLongData"));
    UnprepareLongData
        = reinterpret_cast<UnprepareLongData_t>(GetProcAddress(m_handle, "UnprepareLongData"));
    ReturnKDMAPIVer
        = reinterpret_cast<ReturnKDMAPIVer_t>(GetProcAddress(m_handle, "ReturnKDMAPIVer"));

#if !defined(_MSC_VER)
#pragma GCC diagnostic pop
#endif

    // Check if the minimum required functions are available
    if (!IsKDMAPIAvailable || !InitializeKDMAPIStream || !TerminateKDMAPIStream || !SendDirectData)
    {
      m_available = false;
      return;
    }

    // Call IsKDMAPIAvailable to activate KDMAPI mode in OmniMIDI
    m_available = IsKDMAPIAvailable() != FALSE;
  }

  ~kdmapi_loader()
  {
  }

  kdmapi_loader(const kdmapi_loader&) = delete;
  kdmapi_loader& operator=(const kdmapi_loader&) = delete;

  HMODULE m_handle{};
  bool m_available{false};
};

class kdmapi_stream_manager
{
public:
  static kdmapi_stream_manager& instance()
  {
    static kdmapi_stream_manager mgr;
    return mgr;
  }

  bool acquire()
  {
    std::lock_guard lock{m_mutex};
    auto& loader = kdmapi_loader::instance();
    if (!loader.is_available())
      return false;

    if (m_rc == 0)
    {
      if (!loader.InitializeKDMAPIStream())
        return false;
    }
    ++m_rc;
    return true;
  }

  void release()
  {
    std::lock_guard lock{m_mutex};
    if (m_rc == 0)
      return;

    --m_rc;
    if (m_rc == 0)
    {
      auto& loader = kdmapi_loader::instance();
      if (loader.TerminateKDMAPIStream)
        loader.TerminateKDMAPIStream();
    }
  }

private:
  kdmapi_stream_manager() = default;
  ~kdmapi_stream_manager() = default;
  kdmapi_stream_manager(const kdmapi_stream_manager&) = delete;
  kdmapi_stream_manager& operator=(const kdmapi_stream_manager&) = delete;

  std::mutex m_mutex;
  std::size_t m_rc{0};
};

}
