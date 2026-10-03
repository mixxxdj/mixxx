#pragma once
#include <libremidi/backends/alsa_raw/helpers.hpp>
#include <libremidi/config.hpp>
#include <libremidi/detail/observer.hpp>

#include <cstdio>

NAMESPACE_LIBREMIDI::alsa_raw_ump
{
struct midi2_enumerator : alsa_raw::enumerator
{
  using alsa_raw::enumerator::enumerator;
  void enumerate_devices(int card) override
  {
    using namespace std::literals;
    char name[128];

    sprintf(name, "hw:%d", card);

    // Open card.
    alsa_raw::snd_ctl_wrapper ctl{*this, name};
    if (!ctl)
      return;

    // Enumerate devices.
    int device = -1;
    do
    {
      const int status = snd.ctl.ump.next_device(ctl, &device);
      if (device == -1)
        return;

      if (status < 0)
      {
        handler.libremidi_handle_error(
            configuration, "Cannot determine device number: "s + snd.strerror(status));
        break;
      }

      if (device >= 0)
      {
        enumerate_blocks(ctl, card, device);
        enumerate_endpoints(ctl, card, device);
      }

    } while (device >= 0);
  }

  void enumerate_endpoints(snd_ctl_t* ctl, [[maybe_unused]] int card, [[maybe_unused]] int device)
  {
    snd_ump_endpoint_info_t* info{};
    snd_ump_endpoint_info_alloca(&info);
    snd.ctl.ump.endpoint_info(ctl, info);

    fprintf(stderr, "UMP endpoint: %s", snd.ump.endpoint_info_get_name(info));
  }

  void enumerate_blocks(snd_ctl_t* ctl, [[maybe_unused]] int card, [[maybe_unused]] int device)
  {
    snd_ump_block_info_t* info{};
    snd_ump_block_info_alloca(&info);
    snd.ctl.ump.block_info(ctl, info);

    fprintf(stderr, "UMP block: %s", snd.ump.block_info_get_name(info));
  }
};

}
