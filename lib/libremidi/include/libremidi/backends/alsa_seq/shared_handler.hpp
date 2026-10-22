#pragma once
#if __has_include(<boost/lockfree/spsc_queue.hpp>)
  #include <libremidi/backends/alsa_seq/config.hpp>
  #include <libremidi/backends/alsa_seq/helpers.hpp>
  #include <libremidi/backends/linux/helpers.hpp>
  #include <libremidi/shared_context.hpp>

  #include <boost/lockfree/spsc_queue.hpp>

  #include <variant>

NAMESPACE_LIBREMIDI::alsa_seq
{

struct shared_handler : public libremidi::shared_context
{
  const libasound& snd = libasound::instance();
  struct equals_addr
  {
    constexpr bool operator()(const snd_seq_addr_t& lhs, const snd_seq_addr_t& rhs) noexcept
    {
      return lhs.client == rhs.client && lhs.port == rhs.port;
    }
  };

  explicit shared_handler(std::string_view v)
  {
    if (int err = snd.seq.open(&client, "default", SND_SEQ_OPEN_DUPLEX, SND_SEQ_NONBLOCK); err < 0)
    {
      client = nullptr;
      // fixme throw?
      return;
    }

    if (!v.empty())
      snd.seq.set_client_name(client, v.data());

    // Last descriptor is the eventfd one
    int fds_size = snd.seq.poll_descriptors_count(client, POLLIN);
    fds.reserve(fds_size + 2);
    fds.resize(fds_size);
    snd.seq.poll_descriptors(client, fds.data(), fds_size, POLLIN);
    fds.push_back(termination_event);
    fds.push_back(queue_event);
  }

  void start_processing() override
  {
    thread = std::thread{[this] { process(); }};
  }

  void stop_processing() override
  {
    termination_event.notify();
    if (thread.joinable())
      thread.join();
    termination_event.consume();
  }

  static shared_configurations make(std::string_view client_name)
  {
    auto clt = std::make_shared<shared_handler>(client_name);

    auto cb = [client = std::weak_ptr{clt}](libremidi::alsa_seq::poll_parameters params) {
      if (auto clt = client.lock())
      {
        clt->events.push(
            {.type = shared_handler::event_type::callback_added, .payload = std::move(params)});
        clt->queue_event.notify();
      }
      return true;
    };

    auto stop_cb = [client = std::weak_ptr{clt}](snd_seq_addr_t id) {
      if (auto clt = client.lock())
      {
        clt->events.push({.type = shared_handler::event_type::callback_removed, .payload = id});
        clt->queue_event.notify();
      }
      return true;
    };
    return {
        .context = clt,
        .observer = alsa_seq::
            observer_configuration{.context = clt->client, .manual_poll = cb, .stop_poll = stop_cb},
        .in = alsa_seq::
            input_configuration{.context = clt->client, .manual_poll = cb, .stop_poll = stop_cb},
        .out = alsa_seq::output_configuration{.context = clt->client},
    };
  }

  int64_t index_of_address(snd_seq_addr_t addr)
  {
    auto it = std::find_if(addresses.begin(), addresses.end(), [=](snd_seq_addr_t other) {
      return equals_addr{}(addr, other);
    });
    if (it != addresses.end())
    {
      return std::distance(addresses.begin(), it);
    }
    else
    {
      return -1;
    }
  }

  void process()
  {
    for (;;)
    {
      int err = poll(fds.data(), fds.size(), -1);
      if (err < 0)
        return;
      // Check for termination signal
      if (termination_event.ready(fds[fds.size() - 2]))
        return;

      // Check for queue processing signal
      if (queue_event.ready(fds[fds.size() - 1]))
      {
        this->queue_event.consume();

        event ev;
        while (this->events.pop(ev))
        {
          switch (ev.type)
          {
            case callback_added: {
              auto [addr, cb]
                  = std::move(*get_if<libremidi::alsa_seq::poll_parameters>(&ev.payload));
              addresses.push_back(addr);
              callbacks.push_back(std::move(cb));
              break;
            }
            case callback_removed:
              auto addr = *get_if<snd_seq_addr_t>(&ev.payload);
              if (auto index = index_of_address(addr); index >= 0)
              {
                addresses.erase(addresses.begin() + index);
                callbacks.erase(callbacks.begin() + index);
              }
              break;
          }
        }
      }

      // Look for who's ready
      for (int64_t i = 0, n = std::ssize(fds) - 2; i < n; i++)
      {
        if (fds[i].revents & POLLIN)
        {
          // Read alsa event
          snd_seq_event_t* ev{};
          event_handle handle{snd};
          while (snd.seq.event_input(client, &ev) > 0)
          {
            handle.reset(ev);

            if (auto index = index_of_address(ev->dest); index >= 0)
            {
              // Dispatch the event to the correct observer or midi_in object
              int err = callbacks[index](*ev);
              if (err < 0 && err != -EAGAIN)
                return;
            }
          }
        }
      }
    }
  }

  ~shared_handler() { snd.seq.close(client); }

  enum event_type
  {
    callback_added,
    callback_removed,
  };
  struct event
  {
    event_type type;
    libremidi_variant_alias::variant<libremidi::alsa_seq::poll_parameters, snd_seq_addr_t> payload;
  };

  snd_seq_t* client{};

  boost::lockfree::spsc_queue<event> events{16};
  std::vector<snd_seq_addr_t> addresses;
  std::vector<std::function<int(const snd_seq_event_t&)>> callbacks;
  std::vector<pollfd> fds;
  eventfd_notifier termination_event, queue_event{false};
  std::thread thread;
};
}
#endif
