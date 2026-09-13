#pragma once
#include <libremidi/backends/linux/alsa.hpp>
#include <libremidi/config.hpp>
#include <libremidi/detail/observer.hpp>
#include <libremidi/observer_configuration.hpp>

#include <alsa/asoundlib.h>

#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

// Credits: greatly inspired from
// https://ccrma.stanford.edu/~craig/articles/linuxmidi/alsa-1.0/alsarawmidiout.c
// https://ccrma.stanford.edu/~craig/articles/linuxmidi/alsa-1.0/alsarawportlist.c
// Thanks Craig Stuart Sapp <craig@ccrma.stanford.edu>

NAMESPACE_LIBREMIDI
{
struct alsa_raw_port_id
{
  int card{}, dev{}, port{};
  std::string to_string() const noexcept
  {
    return "hw:" + std::to_string(card) + "," + std::to_string(dev) + "," + std::to_string(port);
  }
};
inline constexpr port_handle raw_to_port_handle(alsa_raw_port_id id) noexcept
{
  return (uint64_t(id.port) << 32) + (uint64_t(id.dev) << 16) + id.card;
}
inline constexpr alsa_raw_port_id raw_from_port_handle(port_handle p) noexcept
{
  alsa_raw_port_id ret;
  ret.port = (p & 0x00'00'FF'FF'00'00'00'00) >> 32;
  ret.dev = (p & 0x00'00'00'00'FF'FF'00'00) >> 16;
  ret.card = (p & 0x00'00'00'00'00'00'FF'FF);
  return ret;
}
static_assert(raw_from_port_handle(raw_to_port_handle({102, 7, 3})).card == 102);
static_assert(raw_from_port_handle(raw_to_port_handle({12, 7, 3})).dev == 7);
static_assert(raw_from_port_handle(raw_to_port_handle({12, 7, 3})).port == 3);

namespace alsa_raw
{
struct alsa_raw_port_info
{
  // hw:1,2,0
  std::string device;
  std::string card_name;
  std::string device_name;
  std::string subdevice_name;
  int card{}, dev{}, sub{};

  std::string pretty_name() const
  {
    return device + ": " + card_name + " : " + device_name + " : " + subdevice_name;
  }

  bool operator==(const alsa_raw_port_info& other) const noexcept = default;
};

struct enumerator;
struct snd_ctl_wrapper
{
  const libasound& snd;
  snd_ctl_t* ctl{};
  inline snd_ctl_wrapper(enumerator& self, const char* name);

  ~snd_ctl_wrapper()
  {
    if (ctl)
    {
      snd.ctl.close(ctl);
    }
  }

  snd_ctl_t& operator*() const noexcept { return *ctl; }
  snd_ctl_t* operator->() const noexcept { return ctl; }
  operator snd_ctl_t*() const noexcept { return ctl; }
};
struct enumerator
{
  const libasound& snd = libasound::instance();
  const error_handler& handler;
  const observer_configuration& configuration;
  std::vector<alsa_raw_port_info> inputs;
  std::vector<alsa_raw_port_info> outputs;

  explicit enumerator(const auto& self)
      : handler{self}
      , configuration{self.configuration}
  {
  }

  // 1: is an input / output
  // 0: isn't an input / output
  // < 0: error
  int is(snd_rawmidi_stream_t stream, snd_ctl_t* ctl, int card, int device, int sub)
  {
    snd_rawmidi_info_t* info;

    snd_rawmidi_info_alloca(&info);
    snd.rawmidi.info_set_device(info, device);
    snd.rawmidi.info_set_subdevice(info, sub);
    snd.rawmidi.info_set_stream(info, stream);

    const int status = snd.ctl.rawmidi.info(ctl, info);
    if (status == 0)
    {
      return 1;
    }
    else if (status < 0 && status != -ENXIO)
    {
      handler.libremidi_handle_error(
          configuration,
          "Cannot get rawmidi information: " + device_identifier(card, device, sub) + " : "
              + snd.strerror(status));
      return status;
    }
    else
    {
      return 0;
    }
  }

  int is_input(snd_ctl_t* ctl, int card, int device, int sub)
  {
    return is(SND_RAWMIDI_STREAM_INPUT, ctl, card, device, sub);
  }

  int is_output(snd_ctl_t* ctl, int card, int device, int sub)
  {
    return is(SND_RAWMIDI_STREAM_OUTPUT, ctl, card, device, sub);
  }

  std::string get_card_name(int card)
  {
    char* card_name{};
    snd.card.get_name(card, &card_name);

    std::string str = card_name;
    free(card_name);
    return str;
  }

  static std::string device_identifier(int card, int device, int sub)
  {
    std::string s;
    s.reserve(12);
    s += "hw:";
    s += std::to_string(card);
    s += ",";
    s += std::to_string(device);
    s += ",";
    s += std::to_string(sub);
    return s;
  }

  stdx::error enumerate_cards()
  {
    using namespace std::literals;
    int card = -1;

    int status = snd.card.next(&card);
    if (status < 0)
    {
      handler.libremidi_handle_error(
          configuration, "Cannot determine card number: "s + snd.strerror(status));
      return from_errc(status);
    }

    if (card < 0)
    {
      handler.libremidi_handle_error(configuration, "No sound cards found");
      return std::errc::no_such_device;
    }

    while (card >= 0)
    {
      enumerate_devices(card);

      if ((status = snd.card.next(&card)) < 0)
      {
        handler.libremidi_handle_error(
            configuration, "cannot determine card number: "s + snd.strerror(status));
        return std::errc::no_such_device;
      }
    }
    return stdx::error{};
  }

  virtual void enumerate_devices(int card) = 0;
};

inline snd_ctl_wrapper::snd_ctl_wrapper(enumerator& self, const char* name)
    : snd{self.snd}
{
  using namespace std::literals;
  int status = snd.ctl.open(&ctl, name, 0);
  if (status < 0)
  {
    self.handler.libremidi_handle_error(
        self.configuration,
        "cannot open control for card"s + name + " : " + snd.strerror(status));
  }
}

struct midi1_enumerator : enumerator
{
  using alsa_raw::enumerator::enumerator;

  void enumerate_devices(int card) override
  {
    using namespace std::literals;
    char name[128];

    sprintf(name, "hw:%d", card);

    // Open card.
    snd_ctl_wrapper ctl{*this, name};
    if (!ctl)
      return;

    // Enumerate devices.
    int device = -1;
    do
    {
      const int status = snd.ctl.rawmidi.next_device(ctl, &device);
      if (device == -1)
        return;

      if (status < 0)
      {
        handler.libremidi_handle_error(
            configuration, "Cannot determine device number: "s + snd.strerror(status));
        break;
      }

      if (device >= 0)
        enumerate_subdevices(ctl, card, device);

    } while (device >= 0);
  }

  void enumerate_subdevices(snd_ctl_t* ctl, int card, int device)
  {
    snd_rawmidi_info_t* info;
    snd_rawmidi_info_alloca(&info);
    snd.rawmidi.info_set_device(info, device);

    snd.rawmidi.info_set_stream(info, SND_RAWMIDI_STREAM_INPUT);
    snd.ctl.rawmidi.info(ctl, info);
    const int subs_in = snd.rawmidi.info_get_subdevices_count(info);

    snd.rawmidi.info_set_stream(info, SND_RAWMIDI_STREAM_OUTPUT);
    snd.ctl.rawmidi.info(ctl, info);
    const int subs_out = snd.rawmidi.info_get_subdevices_count(info);

    alsa_raw_port_info d;
    d.card = card;
    d.dev = device;
    d.card_name = get_card_name(card);
    d.device_name = snd.rawmidi.info_get_name(info);

    auto read_subdevice_info = [&](int sub) {
      snd.rawmidi.info_set_subdevice(info, sub);
      snd.ctl.rawmidi.info(ctl, info);

      d.device = device_identifier(card, device, sub);
      d.subdevice_name = snd.rawmidi.info_get_subdevice_name(info);
      d.sub = sub;
    };

    if (subs_in > 0)
    {
      snd.rawmidi.info_set_stream(info, SND_RAWMIDI_STREAM_INPUT);
      for (int sub = 0; sub < subs_in; sub++)
      {
        read_subdevice_info(sub);
        inputs.push_back(d);
      }
    }

    if (subs_out > 0)
    {
      snd.rawmidi.info_set_stream(info, SND_RAWMIDI_STREAM_OUTPUT);
      for (int sub = 0; sub < subs_out; sub++)
      {
        read_subdevice_info(sub);
        outputs.push_back(d);
      }
    }
  }
};
}
}
