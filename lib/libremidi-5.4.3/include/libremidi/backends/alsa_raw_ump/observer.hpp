#pragma once
#include <libremidi/backends/alsa_raw/observer.hpp>
#include <libremidi/backends/alsa_raw_ump/helpers.hpp>

NAMESPACE_LIBREMIDI::alsa_raw_ump
{
class observer_impl : public alsa_raw::observer_impl_base<midi2_enumerator>
{
public:
  observer_impl(
      libremidi::observer_configuration&& conf, alsa_raw_ump::observer_configuration&& apiconf)
      : observer_impl_base<alsa_raw_ump::midi2_enumerator>{std::move(conf), std::move(apiconf)}
  {
    finish_init();
  }

  libremidi::API get_current_api() const noexcept override { return libremidi::API::ALSA_RAW_UMP; }
};
}
