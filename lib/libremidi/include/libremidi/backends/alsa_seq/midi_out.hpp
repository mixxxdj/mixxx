#pragma once
#include <libremidi/backends/alsa_seq/config.hpp>
#include <libremidi/backends/alsa_seq/helpers.hpp>
#include <libremidi/detail/midi_out.hpp>

NAMESPACE_LIBREMIDI::alsa_seq
{

class midi_out_impl final
    : public midi1::out_api
    , private alsa_data
    , public error_handler
{
public:
  struct
      : libremidi::output_configuration
      , alsa_seq::output_configuration
  {
  } configuration;

  midi_out_impl(libremidi::output_configuration&& conf, alsa_seq::output_configuration&& apiconf)
      : configuration{std::move(conf), std::move(apiconf)}
  {
    if (init_client(configuration) < 0)
    {
      libremidi_handle_error(
          this->configuration,
          "error creating ALSA sequencer client "
          "object.");
      return;
    }

    if (snd.midi.event_new(this->m_bufferSize, &this->coder) < 0)
    {
      libremidi_handle_error(this->configuration, "error initializing MIDI event parser.");
      return;
    }
    snd.midi.event_init(this->coder);

    this->client_open_ = stdx::error{};
  }

  ~midi_out_impl() override
  {
    // Close a connection if it exists.
    midi_out_impl::close_port();

    // Cleanup.
    if (this->vport >= 0)
      snd.seq.delete_port(this->seq, this->vport);
    if (this->coder)
      snd.midi.event_free(this->coder);

    if (!configuration.context)
      snd.seq.close(this->seq);

    client_open_ = std::errc::not_connected;
  }

  libremidi::API get_current_api() const noexcept override { return libremidi::API::ALSA_SEQ; }

  [[nodiscard]] int create_port(std::string_view portName)
  {
    return alsa_data::create_port(
        *this, portName, SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_SUBS_READ,
        SND_SEQ_PORT_TYPE_MIDI_GENERIC | SND_SEQ_PORT_TYPE_APPLICATION, std::nullopt);
  }

  stdx::error open_port(const output_port& p, std::string_view portName) override
  {
    unsigned int n_src
        = this->get_port_count(SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE);
    if (n_src < 1)
    {
      libremidi_handle_error(this->configuration, "no MIDI output sources found!");
      return make_error_code(std::errc::no_such_device);
    }

    auto sink = get_port_info(p);
    if (!sink)
      return std::errc::invalid_argument;

    if (int err = create_port(portName); err < 0)
    {
      libremidi_handle_error(configuration, "ALSA error creating port.");
      return from_errc(err);
    }

    snd_seq_addr_t source{
        .client = (unsigned char)snd.seq.client_id(this->seq), .port = (unsigned char)this->vport};
    if (int err = create_connection(*this, source, *sink, true); err < 0)
    {
      libremidi_handle_error(configuration, "ALSA error making port connection.");
      return from_errc(err);
    }

    return stdx::error{};
  }

  stdx::error open_virtual_port(std::string_view portName) override
  {
    if (int err = create_port(portName); err < 0)
      return from_errc(err);
    return stdx::error{};
  }

  stdx::error close_port() override
  {
    unsubscribe();
    return stdx::error{};
  }

  stdx::error set_port_name(std::string_view portName) override
  {
    return alsa_data::set_port_name(portName);
  }

  stdx::error send_message(const unsigned char* message, std::size_t size) override
  {
    int64_t result{};
    if (size > this->m_bufferSize)
    {
      this->m_bufferSize = size;
      result = snd.midi.event_resize_buffer(this->coder, size);
      if (result != 0)
      {
        libremidi_handle_error(
            this->configuration,
            "ALSA error resizing MIDI event "
            "buffer.");
        return std::errc::no_buffer_space;
      }
    }

    std::size_t offset = 0;
    while (offset < size)
    {
      snd_seq_event_t ev;
      snd_seq_ev_clear(&ev);
      snd_seq_ev_set_source(&ev, this->vport);
      snd_seq_ev_set_subs(&ev);
      // FIXME direct is set but snd_seq_event_output_direct is not used...
      snd_seq_ev_set_direct(&ev);

      const int64_t n_bytes = size; // signed to avoir potential overflow with size - offset below
      result = snd.midi.event_encode(this->coder, message + offset, (long)(n_bytes - offset), &ev);
      if (result < 0)
      {
        libremidi_handle_warning(this->configuration, "event parsing error!");
        return std::errc::bad_message;
      }

      if (ev.type == SND_SEQ_EVENT_NONE)
      {
        libremidi_handle_warning(this->configuration, "incomplete message!");
        return std::errc::message_size;
      }

      offset += result;

      result = snd.seq.event_output(this->seq, &ev);
      if (result < 0)
      {
        libremidi_handle_warning(this->configuration, "error sending MIDI message to port.");
        return std::errc::io_error;
      }
    }
    snd.seq.drain_output(this->seq);
    return stdx::error{};
  }

private:
  uint64_t m_bufferSize{32};
};
}
