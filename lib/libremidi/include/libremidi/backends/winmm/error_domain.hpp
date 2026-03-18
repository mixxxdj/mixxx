#pragma once
// clang-format off
#define NOMINMAX 1
#define WIN32_LEAN_AND_MEAN 1

#include <windows.h>
#include <mmsystem.h>
// clang-format on

#include <libremidi/error.hpp>

NAMESPACE_LIBREMIDI
{

struct winmm_error_domain : public stdx::error_domain
{
public:
  constexpr winmm_error_domain() noexcept
      : error_domain{{0xa32b080ac770514eULL, 0xef59a407f921da43ULL}}
  {
  }

  stdx::string_ref name() const noexcept override { return "winmm"; }

  bool equivalent(const stdx::error& lhs, const stdx::error& rhs) const noexcept override
  {
    if (lhs.domain() == rhs.domain())
      return error_cast<int>(lhs) == error_cast<int>(rhs);

    return false;
  }

  stdx::string_ref message(const stdx::error& e) const noexcept override
  {
    switch (error_cast<int>(e))
    {
      case MMSYSERR_NOERROR:
        return "No error";
      case MMSYSERR_ERROR:
        return "Error";
      case MMSYSERR_BADDEVICEID:
        return "Bad device ID";
      case MMSYSERR_NOTENABLED:
        return "Not enabled";
      case MMSYSERR_ALLOCATED:
        return "Allocated";
      case MMSYSERR_INVALHANDLE:
        return "Invalid handle";
      case MMSYSERR_NODRIVER:
        return "No driver";
      case MMSYSERR_NOMEM:
        return "No memory";
      case MMSYSERR_NOTSUPPORTED:
        return "Not supported";
      case MMSYSERR_BADERRNUM:
        return "Bad errnum";
      case MMSYSERR_INVALFLAG:
        return "Invalid flag";
      case MMSYSERR_INVALPARAM:
        return "Invalid parameter";
      case MMSYSERR_HANDLEBUSY:
        return "Handle busy";
      case MMSYSERR_INVALIDALIAS:
        return "Invalid alias";
      case MMSYSERR_BADDB:
        return "Bad database";
      case MMSYSERR_KEYNOTFOUND:
        return "Key not found";
      case MMSYSERR_READERROR:
        return "Read error";
      case MMSYSERR_WRITEERROR:
        return "Write error";
      case MMSYSERR_DELETEERROR:
        return "Delete error";
      case MMSYSERR_VALNOTFOUND:
        return "Value not found";
      case MMSYSERR_NODRIVERCB:
        return "No driver callback";
      case MMSYSERR_MOREDATA:
        return "More data";
    }
    return "Unknown error code";
  }
};

inline stdx::error from_mmerr(int ret) noexcept
{
  static constexpr winmm_error_domain domain;
  return {ret, domain};
}

}
