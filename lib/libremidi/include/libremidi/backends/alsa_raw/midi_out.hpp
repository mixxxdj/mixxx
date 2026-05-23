#pragma once
#include <libremidi/backends/alsa_raw/config.hpp>
#include <libremidi/backends/alsa_raw/helpers.hpp>
#include <libremidi/detail/midi_out.hpp>

#include <alsa/asoundlib.h>

#include <atomic>
#include <thread>

NAMESPACE_LIBREMIDI::alsa_raw
{
class midi_out_impl final
    : public midi1::out_api
    , public error_handler
{
public:
  struct
      : output_configuration
      , alsa_raw_output_configuration
  {
  } configuration;

  const libasound& snd = libasound::instance();

  midi_out_impl(output_configuration&& conf, alsa_raw_output_configuration&& apiconf)
      : configuration{std::move(conf), std::move(apiconf)}
  {
    client_open_ = stdx::error{};
  }

  ~midi_out_impl() override
  {
    // Close a connection if it exists.
    midi_out_impl::close_port();
    client_open_ = std::errc::not_connected;
  }

  libremidi::API get_current_api() const noexcept override { return libremidi::API::ALSA_RAW; }

  stdx::error connect_port(const char* portname)
  {
    constexpr int mode = SND_RAWMIDI_SYNC;
    int status = snd.rawmidi.open(NULL, &midiport_, portname, mode);
    if (status < 0)
    {
      libremidi_handle_error(this->configuration, "cannot open device.");
      return from_errc(status);
    }
    return stdx::error{};
  }

  stdx::error open_port(const output_port& p, std::string_view) override
  {
    return connect_port(raw_from_port_handle(p.port).to_string().c_str());
  }

  stdx::error close_port() override
  {
    if (midiport_)
      snd.rawmidi.close(midiport_);
    midiport_ = nullptr;

    return stdx::error{};
  }

  stdx::error send_message(const unsigned char* message, size_t size) override
  {
    if (!midiport_)
      libremidi_handle_error(
          this->configuration,
          "trying to send a message without an open "
          "port.");

    if (!this->configuration.chunking)
    {
      return write(message, size);
    }
    else
    {
      return write_chunked(message, size);
    }
  }

  stdx::error write(const unsigned char* message, size_t size)
  {
    if (auto err = snd.rawmidi.write(midiport_, message, size); err < 0)
    {
      libremidi_handle_error(this->configuration, "cannot write message.");
      return from_errc(err);
    }

    return stdx::error{};
  }

  std::size_t get_chunk_size() const noexcept
  {
    snd_rawmidi_params_t* param;
    snd_rawmidi_params_alloca(&param);
    snd.rawmidi.params_current(midiport_, param);

    std::size_t buffer_size = snd.rawmidi.params_get_buffer_size(param);
    return std::min(buffer_size, (std::size_t)configuration.chunking->size);
  }

  std::size_t get_available_bytes_to_write() const noexcept
  {
    snd_rawmidi_status_t* st{};
    snd_rawmidi_status_alloca(&st);
    snd.rawmidi.status(midiport_, st);

    return snd.rawmidi.status_get_avail(st);
  }

  // inspired from ALSA amidi.c source code
  stdx::error write_chunked(const unsigned char* const begin, size_t size)
  {
    const unsigned char* data = begin;
    const unsigned char* end = begin + size;

    const std::size_t chunk_size = std::min(get_chunk_size(), size);

    // Send the first buffer
    std::size_t len = chunk_size;

    if (auto err = write(data, len); err != stdx::error{})
      return err;

    data += len;

    while (data < end)
    {
      // Wait for the buffer to have some space available
      const std::size_t written_bytes = data - begin;
      std::size_t available{};
      while ((available = get_available_bytes_to_write()) < chunk_size)
      {
        if (!configuration.chunking->wait(
                std::chrono::microseconds((chunk_size - available) * 320), written_bytes))
          return std::errc::protocol_error;
      };

      if (!configuration.chunking->wait(configuration.chunking->interval, written_bytes))
        return std::errc::protocol_error;

      // Write more data
      len = end - data;

      // Maybe until the end of the sysex
      if (const auto sysex_end = static_cast<const unsigned char*>(memchr(data, 0xf7, len)))
        len = sysex_end - data + 1;

      if (len > chunk_size)
        len = chunk_size;

      if (auto err = write(data, len); err != stdx::error{})
        return err;

      data += len;
    }

    return stdx::error{};
  }

  snd_rawmidi_t* midiport_{};
};
}
