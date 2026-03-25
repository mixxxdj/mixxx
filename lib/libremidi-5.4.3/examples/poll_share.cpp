#include "utils.hpp"

#include <libremidi/configurations.hpp>
#include <libremidi/libremidi.hpp>

#include <poll.h>

int main()
{
  auto callback = [](const libremidi::message& message) { std::cout << message << std::endl; };

  std::vector<std::function<int(std::span<pollfd>)>> callbacks;
  std::vector<pollfd> fds;

  auto register_fds = [&](const libremidi::manual_poll_parameters& params) {
    fds.insert(fds.end(), params.fds.begin(), params.fds.end());
    callbacks.push_back(params.callback);
    return true;
  };

  libremidi::observer obs{{}, libremidi::alsa_raw_observer_configuration{}};

  // Create as many midi_in as there are connected MIDI sources
  std::vector<libremidi::midi_in> midiin;
  for (auto& port : obs.get_input_ports())
  {
    midiin.emplace_back(
        libremidi::input_configuration{.on_message = callback},
        libremidi::alsa_raw_input_configuration{.manual_poll = register_fds});
    midiin.back().open_port(port);
  }

  for (;;)
  {
    // Option 1:
    // Combine all the fds in your own fd array,
    // and run poll manually
#if 1
    // Poll
    int err = poll(fds.data(), fds.size(), -1);
    if (err < 0)
      return err;

    // Look for who's ready:
    // Note: you have to pass the fds back to the API as the
    // ALSA functions also need to process the fds in addition to poll
    // so you need to keep track of which fds are for which midi_in...
    // In practice it seems that ALSA only uses one FD so it's simply the index
    // in the array but not sure how future-proof this is
    for (int i = 0; i < std::ssize(fds); i++)
    {
      if (fds[i].revents & POLLIN)
      {
        auto err = callbacks[i]({fds.data() + i, 1});
        if (err < 0 && err != -EAGAIN)
          return -err;
      }
    }
#else
    // Option 2:
    // It's also possible to simply pass an empty set of FDs and
    // just process at some custom time interval,
    // in this case no "poll" mechanism is used at all
    for (auto& callback : callbacks)
    {
      auto err = callback({});
      if (err < 0 && err != -EAGAIN)
        return -err;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(16));
#endif
  }
}
