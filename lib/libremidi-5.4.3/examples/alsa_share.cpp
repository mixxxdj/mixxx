#include "utils.hpp"

#include <libremidi/configurations.hpp>
#include <libremidi/detail/memory.hpp>
#include <libremidi/libremidi.hpp>

#include <alsa/asoundlib.h>

#include <poll.h>

constexpr bool operator==(const snd_seq_addr_t& lhs, const snd_seq_addr_t& rhs) noexcept
{
  return lhs.client == rhs.client && lhs.port == rhs.port;
}

int main()
{
  std::vector<libremidi::midi_in> midiin;
  std::vector<libremidi::midi_out> midiout;

  auto callback = [&](int port, const libremidi::message& msg) {
    std::cout << msg << std::endl;
    midiout[port].send_message(msg);
  };

  // Create an alsa client which will be shared across objects
  snd_seq_t* clt{};
  if (int err = snd_seq_open(&clt, "default", SND_SEQ_OPEN_DUPLEX, SND_SEQ_NONBLOCK); err < 0)
    return err;

  int fds_size = snd_seq_poll_descriptors_count(clt, POLLIN);
  auto fds = (struct pollfd*)alloca(fds_size * sizeof(struct pollfd));
  snd_seq_poll_descriptors(clt, fds, fds_size, POLLIN);

  libremidi::unique_handle<snd_seq_t, snd_seq_close> handle{clt};

  snd_seq_set_client_name(clt, "My MIDI app");

  // We share the polling across the midi-ins: see poll_share.cpp for more details
  std::vector<snd_seq_addr_t> addresses;
  std::vector<std::function<int(const snd_seq_event_t&)>> callbacks;

  // Create 16 inputs and 16 outputs
  for (int i = 0; i < 16; i++)
  {
    midiin.emplace_back(
        libremidi::input_configuration{
            .on_message =
                [=](const libremidi::message& msg) {
      std::cout << "Port " << i + 1 << " : ";
      callback(i, msg);
    }},
        libremidi::alsa_seq::input_configuration{
            .context = clt,
            .manual_poll =
                [&addresses, &callbacks](const libremidi::alsa_seq::poll_parameters& params) {
      addresses.push_back(params.addr);
      callbacks.push_back(params.callback);
      return true;
    },
            .stop_poll = [&](snd_seq_addr_t addr) -> bool {
      auto it = std::find(addresses.begin(), addresses.end(), addr);
      auto dist = std::distance(addresses.begin(), it);
      addresses.erase(it);
      callbacks.erase(callbacks.begin() + dist);
      return true;
    }});
    midiin[i].open_virtual_port("Input: " + std::to_string(i + 1));

    midiout.emplace_back(
        libremidi::output_configuration{},
        libremidi::alsa_seq::output_configuration{.context = clt});
    midiout[i].open_virtual_port("Output: " + std::to_string(i + 1));
  }

  // Poll
  for (;;)
  {
    int err = poll(fds, fds_size, -1);
    if (err < 0)
      return err;

    // Look for who's ready
    for (int i = 0; i < fds_size; i++)
    {
      if (fds[i].revents & POLLIN)
      {
        // Read alsa event
        snd_seq_event_t* ev{};
        libremidi::unique_handle<snd_seq_event_t, snd_seq_free_event> handle;
        int result = 0;
        while ((result = snd_seq_event_input(clt, &ev)) > 0)
        {
          handle.reset(ev);
          auto it = std::find(addresses.begin(), addresses.end(), ev->dest);
          if (it != addresses.end())
          {
            // Dispatch the event to the correct observer or midi_in object
            auto index = std::distance(addresses.begin(), it);
            int err = callbacks[index](*ev);
            if (err < 0 && err != -EAGAIN)
              return -err;
          }
        }
      }
    }
  }
}
