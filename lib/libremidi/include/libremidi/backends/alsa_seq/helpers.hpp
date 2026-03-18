#pragma once
#include <libremidi/backends/linux/alsa.hpp>
#include <libremidi/config.hpp>
#include <libremidi/detail/observer.hpp>

#include <sys/time.h>

#include <cstring>
#include <optional>
#include <string>

NAMESPACE_LIBREMIDI::alsa_seq
{
struct event_handle
{
  const libasound& snd;
  snd_seq_event_t* ev{};

  explicit event_handle(const libasound& snd) noexcept
      : snd{snd}
  {
  }

  explicit event_handle(const libasound& snd, snd_seq_event_t* ev) noexcept
      : snd{snd}
      , ev{ev}
  {
  }

  event_handle(const event_handle&) noexcept = delete;
  event_handle& operator=(const event_handle&) noexcept = delete;
  event_handle(event_handle&&) noexcept = delete;
  event_handle& operator=(event_handle&&) noexcept = delete;

  void reset(snd_seq_event_t* new_ev) noexcept
  {
    if (ev)
      snd.seq.free_event(ev);
    ev = new_ev;
  }

  ~event_handle() { snd.seq.free_event(ev); }
};

inline constexpr port_handle seq_to_port_handle(uint64_t client, uint64_t port) noexcept
{
  return (port << 32) + client;
}

inline constexpr std::pair<int, int> seq_from_port_handle(port_handle p) noexcept
{
  int port = p >> 32;
  int client = p & 0xFFFFFFFF;
  return {client, port};
}

inline void for_all_clients(
    const libasound& snd, snd_seq_t* seq, const std::function<void(snd_seq_client_info_t&)>& func)
{
  snd_seq_client_info_t* cinfo{};
  snd_seq_client_info_alloca(&cinfo);
  snd_seq_port_info_t* pinfo{};
  snd_seq_port_info_alloca(&pinfo);

  snd.seq.client_info_set_client(cinfo, -1);
  while (snd.seq.query_next_client(seq, cinfo) >= 0)
  {
    int client = snd.seq.client_info_get_client(cinfo);
    if (client == 0)
      continue;
    func(*cinfo);
  }
}

inline void for_all_ports(
    const libasound& snd, snd_seq_t* seq,
    const std::function<void(snd_seq_client_info_t&, snd_seq_port_info_t&)>& func)
{
  snd_seq_client_info_t* cinfo{};
  snd_seq_client_info_alloca(&cinfo);
  snd_seq_port_info_t* pinfo{};
  snd_seq_port_info_alloca(&pinfo);

  snd.seq.client_info_set_client(cinfo, -1);
  while (snd.seq.query_next_client(seq, cinfo) >= 0)
  {
    int client = snd.seq.client_info_get_client(cinfo);
    if (client == 0)
      continue;

    // Reset query info
    snd.seq.port_info_set_client(pinfo, client);
    snd.seq.port_info_set_port(pinfo, -1);
    while (snd.seq.query_next_port(seq, pinfo) >= 0)
    {
      func(*cinfo, *pinfo);
    }
  }
}

inline void for_all_ports(
    const libasound& snd, snd_seq_t* seq, int client,
    const std::function<void(snd_seq_port_info_t&)>& func)
{
  snd_seq_port_info_t* pinfo{};
  snd_seq_port_info_alloca(&pinfo);

  snd.seq.port_info_set_client(pinfo, client);
  snd.seq.port_info_set_port(pinfo, -1);
  while (snd.seq.query_next_port(seq, pinfo) >= 0)
  {
    func(*pinfo);
  }
}

// This function is used to count or get the pinfo structure for a given port
// number.
inline unsigned int iterate_port_info(
    const libasound& snd, snd_seq_t* seq, snd_seq_port_info_t* pinfo, unsigned int type,
    int portNumber)
{
  snd_seq_client_info_t* cinfo{};
  int count = 0;
  snd_seq_client_info_alloca(&cinfo);

  snd.seq.client_info_set_client(cinfo, -1);
  while (snd.seq.query_next_client(seq, cinfo) >= 0)
  {
    const int client = snd.seq.client_info_get_client(cinfo);
    if (client == 0)
      continue;

    // Reset query info
    snd.seq.port_info_set_client(pinfo, client);
    snd.seq.port_info_set_port(pinfo, -1);
    while (snd.seq.query_next_port(seq, pinfo) >= 0)
    {
      const unsigned int atyp = snd.seq.port_info_get_type(pinfo);
      if (((atyp & SND_SEQ_PORT_TYPE_MIDI_GENERIC) == 0) && ((atyp & SND_SEQ_PORT_TYPE_SYNTH) == 0)
          && ((atyp & SND_SEQ_PORT_TYPE_APPLICATION) == 0))
        continue;

      const unsigned int caps = snd.seq.port_info_get_capability(pinfo);
      if ((caps & type) != type)
        continue;
      if ((caps & SND_SEQ_PORT_CAP_NO_EXPORT) != 0)
        continue;
      if (count == portNumber)
        return 1;
      ++count;
    }
  }

  // If a negative portNumber was used, return the port count.
  if (portNumber < 0)
    return count;
  return 0;
}

// A structure to hold variables related to the ALSA API
// implementation.
struct alsa_data
{
  const libasound& snd = libasound::instance();
  snd_seq_t* seq{};
  int vport{-1};
  snd_seq_addr_t vaddr{};

  snd_seq_port_subscribe_t* subscription{};
  snd_midi_event_t* coder{};

  [[nodiscard]] int init_client(auto& configuration)
  {
    // Initialize or use the snd_seq client
    if (configuration.context)
    {
      seq = configuration.context;
      return 0;
    }
    else
    {
      // Set up the ALSA sequencer client.
      int ret = snd.seq.open(&seq, "default", SND_SEQ_OPEN_DUPLEX, SND_SEQ_NONBLOCK);
      if (ret < 0)
        return ret;

      // Set client name.
      if (!configuration.client_name.empty())
        snd.seq.set_client_name(seq, configuration.client_name.data());

#if __has_include(<alsa/ump.h>)
      if (snd.seq.set_client_midi_version)
      {
        switch (configuration.midi_version)
        {
          case 1:
            snd.seq.set_client_midi_version(seq, SND_SEQ_CLIENT_LEGACY_MIDI);
            break;
          case 2:
            snd.seq.set_client_midi_version(seq, SND_SEQ_CLIENT_UMP_MIDI_2_0);
            break;
        }
      }
#endif

      return 0;
    }
  }

  stdx::error set_client_name(std::string_view clientName)
  {
    int ret = snd.seq.set_client_name(seq, clientName.data());
    return from_errc(ret);
  }

  stdx::error set_port_name(std::string_view portName)
  {
    snd_seq_port_info_t* pinfo;
    snd_seq_port_info_alloca(&pinfo);
    snd.seq.get_port_info(seq, vport, pinfo);
    snd.seq.port_info_set_name(pinfo, portName.data());
    int ret = snd.seq.set_port_info(seq, vport, pinfo);
    return from_errc(ret);
  }

  unsigned int get_port_count(int caps) const
  {
    snd_seq_port_info_t* pinfo;
    snd_seq_port_info_alloca(&pinfo);

    return alsa_seq::iterate_port_info(snd, seq, pinfo, caps, -1);
  }

  std::optional<snd_seq_addr_t> get_port_info(const port_information& portNumber)
  {
    auto [client, port] = alsa_seq::seq_from_port_handle(portNumber.port);
    // FIXME check that the {client, port} pair actually exists
    // snd_seq_port_info_t* src_pinfo{};
    // snd_seq_port_info_alloca(&src_pinfo);
    // snd.seq.port_info_set_client(src_pinfo, client);
    // snd.seq.port_info_set_port(src_pinfo, port);

    // {
    //   self.libremidi_handle_error(
    //       self.configuration,
    //       "invalid 'portNumber' argument: " + std::to_string(portNumber));
    //   return {};
    // }
    snd_seq_addr_t addr;
    addr.client = client;
    addr.port = port;
    return addr;
  }

  [[nodiscard]] int create_port(
      auto& /*self*/, std::string_view portName, unsigned int caps, unsigned int type,
      std::optional<int> queue)
  {
    if (this->vport < 0)
    {
      snd_seq_port_info_t* pinfo{};
      snd_seq_port_info_alloca(&pinfo);

      snd.seq.port_info_set_name(pinfo, portName.data());
      snd.seq.port_info_set_client(pinfo, 0);
      snd.seq.port_info_set_port(pinfo, 0);
      snd.seq.port_info_set_capability(pinfo, caps);
      snd.seq.port_info_set_type(pinfo, type);

      if (type & SND_SEQ_PORT_TYPE_MIDI_GENERIC)
      {
        snd.seq.port_info_set_midi_channels(pinfo, 16);
      }

      if (queue)
      {
        snd.seq.port_info_set_timestamping(pinfo, 1);
        snd.seq.port_info_set_timestamp_real(pinfo, 1);
        snd.seq.port_info_set_timestamp_queue(pinfo, *queue);
      }

      if (int err = snd.seq.create_port(this->seq, pinfo); err < 0)
        return err;

      this->vport = snd.seq.port_info_get_port(pinfo);
      if (int err = snd.seq.get_port_info(this->seq, this->vport, pinfo); err < 0)
        return err;

      if (auto addr = snd.seq.port_info_get_addr(pinfo))
        this->vaddr = *addr;
      else
        return -1;

      return this->vport;
    }
    return 0;
  }

  int create_connection(auto& self, snd_seq_addr_t sender, snd_seq_addr_t receiver, bool realtime)
  {
    // Create the connection between ports
    // Make subscription
    if (int err = snd.seq.port_subscribe_malloc(&this->subscription); err < 0)
    {
      self.libremidi_handle_error(self.configuration, "ALSA error allocation port subscription.");
      return err;
    }

    snd.seq.port_subscribe_set_sender(this->subscription, &sender);
    snd.seq.port_subscribe_set_dest(this->subscription, &receiver);

    if (realtime)
    {
      snd.seq.port_subscribe_set_time_update(this->subscription, 1);
      snd.seq.port_subscribe_set_time_real(this->subscription, 1);
    }

    if (int err = snd.seq.subscribe_port(this->seq, this->subscription); err != 0)
    {
      snd.seq.port_subscribe_free(this->subscription);
      this->subscription = nullptr;
      return err;
    }
    return 0;
  }

  void unsubscribe()
  {
    if (this->subscription)
    {
      snd.seq.unsubscribe_port(this->seq, this->subscription);
      snd.seq.port_subscribe_free(this->subscription);
      this->subscription = nullptr;
    }
  }
};
}
