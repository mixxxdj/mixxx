#pragma once
#include <libremidi/error.hpp>

#if __has_include(<weakjack/weak_libjack.h>)
  #include <weakjack/weak_libjack.h>
#elif __has_include(<weak_libjack.h>)
  #include <weak_libjack.h>
#elif __has_include(<jack/jack.h> )
  #include <jack/jack.h>
  #include <jack/midiport.h>
  #include <jack/ringbuffer.h>
#endif

NAMESPACE_LIBREMIDI
{
struct jack_error_domain : public stdx::error_domain
{
public:
  constexpr jack_error_domain() noexcept
      : error_domain{{0xc714ea32b705080aULL, 0xe409a437daf5f921ULL}}
  {
  }

  stdx::string_ref name() const noexcept override { return "jack"; }

  bool equivalent(const stdx::error& lhs, const stdx::error& rhs) const noexcept override
  {
    if (lhs.domain() == rhs.domain())
      return error_cast<jack_status_t>(lhs) == error_cast<jack_status_t>(rhs);

    return false;
  }

  stdx::string_ref message(const stdx::error& e) const noexcept override
  {
    const auto status = error_cast<jack_status_t>(e);
    if (status == jack_status_t{})
      return "Success";

    if (status & JackInvalidOption)
      return "The operation contained an invalid or unsupported option";
    if (status & JackServerFailed)
      return "Unable to connect to the JACK server";
    if (status & JackServerError)
      return "Communication error with the JACK server";
    if (status & JackNoSuchClient)
      return "Requested client does not exist";
    if (status & JackLoadFailure)
      return "Unable to load internal client";
    if (status & JackInitFailure)
      return "Unable to initialize client";
    if (status & JackShmFailure)
      return "Unable to access shared memory";
    if (status & JackVersionError)
      return "Client's protocol version does not match";
    if (status & JackBackendError)
      return "Backend error";
    if (status & JackClientZombie)
      return "Client zombified failure";
    if (status & JackFailure)
      return "Failure";
    if (status & JackNameNotUnique)
      return "The desired client name was not unique";

    // Can't happen in libremidi as we set JackNoStartServer
    if (status & JackServerStarted)
      return "Server was started";

    return "Unknown JACK status code";
  }
};

inline stdx::error from_jack_status(jack_status_t ret) noexcept
{
  static constexpr jack_error_domain domain{};
  return {ret, domain};
}
}
