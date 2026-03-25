#pragma once
#include <libremidi/backends/jack/config.hpp>
#include <libremidi/backends/jack/helpers.hpp>
#include <libremidi/detail/midi_in.hpp>
#include <libremidi/detail/midi_stream_decoder.hpp>

NAMESPACE_LIBREMIDI
{
class midi_in_jack final
    : public midi1::in_api
    , public jack_helpers
    , public jack_midi1
    , public error_handler
{
public:
  using midi_api::client_open_;
  struct
      : input_configuration
      , jack_input_configuration
  {
  } configuration;

  explicit midi_in_jack(input_configuration&& conf, jack_input_configuration&& apiconf)
      : configuration{std::move(conf), std::move(apiconf)}
  {
    auto status = connect(*this);
    if (!this->client)
    {
      libremidi_handle_error(configuration, "Could not create JACK client");
      client_open_ = from_jack_status(status);
      return;
    }

    client_open_ = stdx::error{};
  }

  ~midi_in_jack() override
  {
    midi_in_jack::close_port();

    disconnect(*this);
  }

  libremidi::API get_current_api() const noexcept override { return libremidi::API::JACK_MIDI; }

  stdx::error open_port(const input_port& port, std::string_view portName) override
  {
    if (auto err = create_local_port(*this, portName, port_type, JackPortIsInput);
        err != stdx::error{})
      return err;

    if (int err = jack_connect(this->client, port.port_name.c_str(), jack_port_name(this->port));
        err != 0 && err != EEXIST)
    {
      libremidi_handle_error(
          configuration,
          "could not connect to port: " + port.port_name + " -> " + jack_port_name(this->port));
      return from_errc(err);
    }
    return stdx::error{};
  }

  stdx::error open_virtual_port(std::string_view portName) override
  {
    return create_local_port(*this, portName, port_type, JackPortIsInput);
  }

  stdx::error close_port() override { return do_close_port(); }

  stdx::error set_port_name(std::string_view portName) override
  {
    int ret = jack_port_rename(this->client, this->port, portName.data());
    return from_errc(ret);
  }

  timestamp absolute_timestamp() const noexcept override
  {
    return 1000 * jack_frames_to_time(client, jack_frame_time(client));
  }

  int process(jack_nframes_t nframes)
  {
    static constexpr timestamp_backend_info timestamp_info{
        .has_absolute_timestamps = true,
        .absolute_is_monotonic = true,
        .has_samples = true,
    };
    void* buff = jack_port_get_buffer(this->port, nframes);

    // Timing
    jack_nframes_t current_frames{};
    jack_time_t current_usecs{}; // roughly CLOCK_MONOTONIC
    jack_time_t next_usecs{};
    float period_usecs{};
    jack_get_cycle_times(
        this->client, &current_frames, &current_usecs, &next_usecs, &period_usecs);

    // We have midi events in buffer
    uint32_t ev_count = jack_midi_get_event_count(buff);
    for (uint32_t j = 0; j < ev_count; j++)
    {
      jack_midi_event_t event{};
      jack_midi_event_get(&event, buff, j);
      const auto to_ns
          = [=, this] { return 1000 * jack_frames_to_time(client, current_frames + event.time); };

      m_processing.on_bytes(
          {event.buffer, event.buffer + event.size},
          m_processing.timestamp<timestamp_info>(to_ns, event.time));
    }

    return 0;
  }

  midi1::input_state_machine m_processing{this->configuration};
};
}
