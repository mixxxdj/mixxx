#pragma once
#include <libremidi/backends/winmm/error_domain.hpp>
#include <libremidi/detail/midi_api.hpp>

#include <string>

NAMESPACE_LIBREMIDI
{

// Convert a nullptr-terminated wide string or ANSI-encoded string to UTF-8.
inline std::string ConvertToUTF8(const TCHAR* str)
{
  std::string u8str;
  const WCHAR* wstr = L"";
#if defined(UNICODE) || defined(_UNICODE)
  wstr = str;
#else
  // Convert from ANSI encoding to wide string
  int wlength = MultiByteToWideChar(CP_ACP, 0, str, -1, nullptr, 0);
  std::wstring wstrtemp;
  if (wlength)
  {
    wstrtemp.assign(wlength - 1, 0);
    MultiByteToWideChar(CP_ACP, 0, str, -1, &wstrtemp[0], wlength);
    wstr = &wstrtemp[0];
  }
#endif
  // Convert from wide string to UTF-8
  int length = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, nullptr, 0, nullptr, nullptr);
  if (length)
  {
    u8str.assign(static_cast<std::string::size_type>(length - 1), 0);
    /*length =*/WideCharToMultiByte(CP_UTF8, 0, wstr, -1, &u8str[0], length, nullptr, nullptr);
  }
  return u8str;
}

// Next functions add the portNumber to the name so that
// the device's names are sure to be listed with individual names
// even when they have the same brand name
inline void MakeUniqueInPortName(std::string& deviceName, std::size_t portNumber)
{
  int x = 1;
  for (std::size_t i = 0; i < portNumber; i++)
  {
    MIDIINCAPS deviceCaps;
    midiInGetDevCaps(i, &deviceCaps, sizeof(MIDIINCAPS));
    auto stringName = ConvertToUTF8(deviceCaps.szPname);
    if (deviceName == stringName)
    {
      x++;
    }
  }
  deviceName += " ";
  deviceName += std::to_string(x);
}

inline void MakeUniqueOutPortName(std::string& deviceName, std::size_t portNumber)
{
  int x = 1;
  for (std::size_t i = 0; i < portNumber; i++)
  {
    MIDIOUTCAPS deviceCaps;
    midiOutGetDevCaps(i, &deviceCaps, sizeof(MIDIOUTCAPS));
    auto stringName = ConvertToUTF8(deviceCaps.szPname);
    if (deviceName == stringName)
    {
      x++;
    }
  }
  deviceName += " ";
  deviceName += std::to_string(x);
}
}
