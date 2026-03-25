#pragma once
#include <libremidi/backends/jack/config.hpp>
#include <libremidi/backends/jack/helpers.hpp>
#include <libremidi/detail/midi_out.hpp>

NAMESPACE_LIBREMIDI
{
class midi_out_jack
    : public midi1::out_api
    , public jack_helpers
    , public jack_midi1
    , public error_handler
{
public:
  using midi_api::client_open_;
  struct
      : output_configuration
      , jack_output_configuration
  {
  } configuration;

  midi_out_jack(output_configuration&& conf, jack_output_configuration&& apiconf)
      : configuration{std::move(conf), std::move(apiconf)}
  {
  }

  ~midi_out_jack() override { }

  libremidi::API get_current_api() const noexcept override { return libremidi::API::JACK_MIDI; }

  stdx::error open_port(const output_port& port, std::string_view portName) override
  {
    if (auto err = create_local_port(*this, portName, port_type, JackPortIsOutput);
        err != stdx::error{})
      return err;

    // Connecting to the output
    if (int err = jack_connect(this->client, jack_port_name(this->port), port.port_name.c_str());
        err != 0 && err != EEXIST)
    {
      libremidi_handle_error(configuration, "could not connect to port" + port.port_name);
      return from_errc(err);
    }

    return stdx::error{};
  }

  stdx::error open_virtual_port(std::string_view portName) override
  {
    return create_local_port(*this, portName, port_type, JackPortIsOutput);
  }

  stdx::error close_port() override { return do_close_port(); }

  stdx::error set_port_name(std::string_view portName) override
  {
    int ret = jack_port_rename(this->client, this->port, portName.data());
    return from_errc(ret);
  }
};

class midi_out_jack_queued final : public midi_out_jack
{
public:
  midi_out_jack_queued(output_configuration&& conf, jack_output_configuration&& apiconf)
      : midi_out_jack{std::move(conf), std::move(apiconf)}
      , m_queue{configuration.ringbuffer_size}
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

  ~midi_out_jack_queued() override
  {
    midi_out_jack::close_port();

    disconnect(*this);
  }

  stdx::error send_message(const unsigned char* message, std::size_t size) override
  {
    return m_queue.write(message, size);
  }

  int process(jack_nframes_t nframes)
  {
    void* buff = jack_port_get_buffer(this->port, nframes);
    jack_midi_clear_buffer(buff);

    this->m_queue.read(buff);

    return 0;
  }

private:
  jack_queue m_queue;
};

class midi_out_jack_direct final : public midi_out_jack
{
public:
  midi_out_jack_direct(output_configuration&& conf, jack_output_configuration&& apiconf)
      : midi_out_jack{std::move(conf), std::move(apiconf)}
  {
    auto status = connect(*this);
    if (!this->client)
    {
      libremidi_handle_error(configuration, "Could not create JACK client");
      client_open_ = from_jack_status(status);
      return;
    }

    buffer_size = jack_get_buffer_size(this->client);
    client_open_ = stdx::error{};
  }

  ~midi_out_jack_direct() override
  {
    midi_out_jack::close_port();

    disconnect(*this);
  }

  int process(jack_nframes_t nframes)
  {
    void* buff = jack_port_get_buffer(this->port, nframes);
    jack_midi_clear_buffer(buff);
    return 0;
  }

  stdx::error send_message(const unsigned char* message, size_t size) override
  {
    void* buff = jack_port_get_buffer(this->port, buffer_size);
    int ret = jack_midi_event_write(buff, 0, message, size);
    return from_errc(ret);
  }

  int convert_timestamp(int64_t user) const noexcept
  {
    switch (configuration.timestamps)
    {
      case timestamp_mode::AudioFrame:
        return static_cast<int>(user);

      default:
        // TODO
        return 0;
    }
  }

  stdx::error schedule_message(int64_t ts, const unsigned char* message, size_t size) override
  {
    void* buff = jack_port_get_buffer(this->port, buffer_size);
    int ret = jack_midi_event_write(buff, convert_timestamp(ts), message, size);
    return from_errc(ret);
  }

  int buffer_size{};
};
}

NAMESPACE_LIBREMIDI
{
template <>
inline std::unique_ptr<midi_out_api> make<midi_out_jack>(
    libremidi::output_configuration&& conf, libremidi::jack_output_configuration&& api)
{
  if (api.direct)
    return std::make_unique<midi_out_jack_direct>(std::move(conf), std::move(api));
  else
    return std::make_unique<midi_out_jack_queued>(std::move(conf), std::move(api));
}
}
