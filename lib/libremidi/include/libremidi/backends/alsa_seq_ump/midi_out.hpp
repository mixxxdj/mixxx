#pragma once
#include <libremidi/backends/alsa_seq/helpers.hpp>
#include <libremidi/backends/alsa_seq_ump/config.hpp>
#include <libremidi/backends/alsa_seq_ump/helpers.hpp>
#include <libremidi/detail/midi_out.hpp>
#include <libremidi/detail/ump_stream.hpp>

NAMESPACE_LIBREMIDI::alsa_seq_ump
{

class midi_out_impl final
    : public midi2::out_api
    , private alsa_seq::alsa_data
    , public error_handler
{
public:
  struct
      : libremidi::output_configuration
      , alsa_seq_ump::output_configuration
  {
  } configuration;

  midi_out_impl(
      libremidi::output_configuration&& conf, alsa_seq_ump::output_configuration&& apiconf)
      : configuration{std::move(conf), std::move(apiconf)}
  {
    assert(snd.seq.ump.available);
    if (init_client(configuration) < 0)
    {
      libremidi_handle_error(
          this->configuration,
          "error creating ALSA sequencer client "
          "object.");
      return;
    }

    this->client_open_ = stdx::error{};
  }

  ~midi_out_impl() override
  {
    // Close a connection if it exists.
    midi_out_impl::close_port();

    // Cleanup.
    if (this->vport >= 0)
      snd.seq.delete_port(this->seq, this->vport);

    if (!configuration.context)
      snd.seq.close(this->seq);

    client_open_ = std::errc::not_connected;
  }

  libremidi::API get_current_api() const noexcept override { return libremidi::API::ALSA_SEQ_UMP; }

  [[nodiscard]] int create_port(std::string_view portName)
  {
    return alsa_data::create_port(
        *this, portName,
        SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_SUBS_READ | SND_SEQ_PORT_CAP_UMP_ENDPOINT,
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

  stdx::error send_ump(const uint32_t* ump_stream, std::size_t count) override
  {
    snd_seq_ump_event_t ev;

    memset(&ev, 0, sizeof(snd_seq_ump_event_t));
    snd_seq_ev_set_ump(&ev);
    snd_seq_ev_set_source(&ev, this->vport);
    snd_seq_ev_set_subs(&ev);
    snd_seq_ev_set_direct(&ev);

    auto write_func = [this, &ev](const uint32_t* ump, int64_t bytes) -> std::errc {
      std::memcpy(ev.ump, ump, bytes);
      const int ret = snd.seq.ump.event_output_direct(this->seq, &ev);
      if (ret < 0)
      {
        libremidi_handle_warning(this->configuration, "error sending MIDI message to port.");
        return static_cast<std::errc>(-ret);
      }
      static_assert(std::errc{0} == std::errc{});
      return std::errc{};
    };
    segment_ump_stream(ump_stream, count, write_func, []() { });

    snd.seq.drain_output(this->seq);
    return stdx::error{};
  }
};
}
