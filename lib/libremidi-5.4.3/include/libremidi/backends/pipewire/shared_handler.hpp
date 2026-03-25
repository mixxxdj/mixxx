#pragma once
#if __has_include(<boost/lockfree/spsc_queue.hpp>)
  #include <libremidi/backends/pipewire/config.hpp>
  #include <libremidi/backends/pipewire/helpers.hpp>
  #include <libremidi/shared_context.hpp>

  #include <boost/lockfree/spsc_queue.hpp>

  #include <variant>

NAMESPACE_LIBREMIDI::pipewire
{

// Create a PipeWire client which will be shared across objects
struct shared_handler : public libremidi::shared_context
{
  explicit shared_handler(std::string_view v)
  {
    midiin_callbacks.reserve(64);
    midiout_callbacks.reserve(64);

    pipewire_status_t status{};
    client = pipewire_client_open(v.data(), PipewireNoStartServer, &status);
    assert(client);
    assert(status == 0);
    pipewire_set_process_callback(client, +[](pipewire_nframes_t cnt, void* ctx) -> int {
      ((shared_handler*)ctx)->pipewire_callback(cnt);
      return 0;
    }, this);
  }

  virtual void start_processing() override { pipewire_activate(client); }
  virtual void stop_processing() override { pipewire_deactivate(client); }

  static shared_configurations make(std::string_view client_name)
  {
    auto clt = std::make_shared<shared_handler>(client_name);
    auto add_in_cb = [client = std::weak_ptr{clt}](libremidi::pipewire_callback cb) {
      if (auto clt = client.lock())
        clt->events.push({shared_handler::event_type::in_callback_added, std::move(cb)});
    };
    auto clear_in_cb = [client = std::weak_ptr{clt}](int64_t index) {
      if (auto clt = client.lock())
        clt->events.push({shared_handler::event_type::in_callback_removed, index});
    };
    auto add_out_cb = [client = std::weak_ptr{clt}](libremidi::pipewire_callback cb) {
      if (auto clt = client.lock())
        clt->events.push({shared_handler::event_type::out_callback_added, std::move(cb)});
    };
    auto clear_out_cb = [client = std::weak_ptr{clt}](int64_t index) {
      if (auto clt = client.lock())
        clt->events.push({shared_handler::event_type::out_callback_removed, index});
    };
    return {
        .context = clt,
        .observer = pipewire_observer_configuration{.context = clt->client},
        .in
        = pipewire_input_configuration{.context = clt->client, .set_process_func = add_in_cb, .clear_process_func = clear_in_cb},
        .out
        = pipewire_output_configuration{.context = clt->client, .set_process_func = add_out_cb, .clear_process_func = clear_out_cb},
    };
  }

  int pipewire_callback(pipewire_nframes_t cnt)
  {
    // 1. Process the events that will change the callback list
    event ev;
    while (events.pop(ev))
    {
      switch (ev.type)
      {
        case in_callback_added:
          midiin_callbacks.push_back(
              std::move(*get_if<libremidi::pipewire_callback>(&ev.payload)));
          break;
        case in_callback_removed: {
          auto idx = *get_if<int64_t>(&ev.payload);
          for (auto it = midiin_callbacks.begin(); it != midiin_callbacks.end();)
          {
            if (it->token == idx)
            {
              midiin_callbacks.erase(it);
              break;
            }
            else
            {
              ++it;
            }
          }
          break;
        }
        case out_callback_added:
          midiout_callbacks.push_back(
              std::move(*get_if<libremidi::pipewire_callback>(&ev.payload)));
          break;
        case out_callback_removed:
          auto idx = *get_if<int64_t>(&ev.payload);
          for (auto it = midiout_callbacks.begin(); it != midiout_callbacks.end();)
          {
            if (it->token == idx)
            {
              midiout_callbacks.erase(it);
              break;
            }
            else
            {
              ++it;
            }
          }
          break;
      }
    }

    for (auto& cb : midiin_callbacks)
      cb.callback(cnt);

    for (auto& cb : midiout_callbacks)
      cb.callback(cnt);

    return 0;
  }

  ~shared_handler()
  {
    pipewire_deactivate(client);
    pipewire_client_close(client);
  }

  pipewire_client_t* client{};

  enum event_type
  {
    in_callback_added,
    in_callback_removed,
    out_callback_added,
    out_callback_removed,
  };
  struct event
  {
    event_type type;
    libremidi_variant_alias::variant<libremidi::pipewire_callback, int64_t> payload;
  };

  boost::lockfree::spsc_queue<event> events{16};

  std::vector<libremidi::pipewire_callback> midiin_callbacks;
  std::vector<libremidi::pipewire_callback> midiout_callbacks;
};
}
#endif
