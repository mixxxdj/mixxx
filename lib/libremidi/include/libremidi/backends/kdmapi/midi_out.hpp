#pragma once
#include <libremidi/backends/kdmapi/config.hpp>
#include <libremidi/backends/kdmapi/helpers.hpp>
#include <libremidi/backends/kdmapi/observer.hpp>
#include <libremidi/detail/midi_out.hpp>

namespace libremidi::kdmapi
{

class midi_out final
    : public midi1::out_api
    , public error_handler
{
public:
  struct
      : libremidi::output_configuration
      , kdmapi::output_configuration
  {
  } configuration;

  midi_out(libremidi::output_configuration&& conf, kdmapi::output_configuration&& apiconf)
      : configuration{std::move(conf), std::move(apiconf)}
  {
    auto& loader = kdmapi_loader::instance();
    if (!loader.is_available())
    {
      libremidi_handle_error(
          configuration, "KDMAPI is not available. Is OmniMIDI installed?");
      this->client_open_ = std::errc::no_such_device;
      return;
    }

    this->client_open_ = stdx::error{};
  }

  ~midi_out() override
  {
    midi_out::close_port();
    this->client_open_ = std::errc::not_connected;
  }

  libremidi::API get_current_api() const noexcept override { return libremidi::API::KDMAPI; }

  stdx::error open_port(const output_port& port, std::string_view) override
  {
    if (port.api != libremidi::API::KDMAPI)
    {
      libremidi_handle_error(configuration, "port is not a KDMAPI port");
      return std::errc::invalid_argument;
    }

    return do_open();
  }

  stdx::error do_open()
  {
    auto& mgr = kdmapi_stream_manager::instance();
    if (!mgr.acquire())
    {
      libremidi_handle_error(configuration, "failed to initialize KDMAPI stream");
      return std::errc::io_error;
    }

    connected_ = true;
    return stdx::error{};
  }

  stdx::error close_port() override
  {
    if (connected_)
    {
      auto& mgr = kdmapi_stream_manager::instance();
      mgr.release();
      connected_ = false;
    }
    return stdx::error{};
  }

  stdx::error send_message(const unsigned char* message, size_t size) override
  {
    if (!connected_)
      return std::errc::not_connected;

    if (size == 0)
    {
      libremidi_handle_warning(configuration, "message argument is empty!");
      return std::errc::invalid_argument;
    }

    auto& loader = kdmapi_loader::instance();

    if (message[0] == 0xF0)
    {
      // SysEx message
      return send_sysex(message, size);
    }
    else
    {
      // Channel or system message
      if (size > 3)
      {
        libremidi_handle_warning(
            configuration, "message size is greater than 3 bytes (and not sysex)!");
        return std::errc::message_size;
      }

      // Pack MIDI bytes into DWORD
      DWORD packet = 0;
      std::copy_n(message, size, reinterpret_cast<unsigned char*>(&packet));

      // Send the message
      if (configuration.use_no_buffer && loader.SendDirectDataNoBuf)
        loader.SendDirectDataNoBuf(packet);
      else
        loader.SendDirectData(packet);
    }

    return stdx::error{};
  }

private:
  stdx::error send_sysex(const unsigned char* message, size_t size)
  {
    auto& loader = kdmapi_loader::instance();

    if (configuration.use_no_buffer && loader.SendDirectLongDataNoBuf)
    {
      // Use the no-buffer variant - it takes raw data directly
      loader.SendDirectLongDataNoBuf(
          const_cast<LPSTR>(reinterpret_cast<const char*>(message)), static_cast<DWORD>(size));
      return stdx::error{};
    }

    // Standard path with MIDIHDR
    buffer_.assign(message, message + size);

    MIDIHDR sysex{};
    sysex.lpData = reinterpret_cast<LPSTR>(buffer_.data());
    sysex.dwBufferLength = static_cast<DWORD>(size);
    sysex.dwFlags = 0;

    if (loader.PrepareLongData)
    {
      UINT result = loader.PrepareLongData(&sysex, sizeof(MIDIHDR));
      if (result != MMSYSERR_NOERROR)
      {
        libremidi_handle_error(configuration, "error preparing sysex header");
        return std::errc::io_error;
      }
    }

    if (loader.SendDirectLongData)
    {
      UINT result = loader.SendDirectLongData(&sysex, sizeof(MIDIHDR));
      if (result != MMSYSERR_NOERROR)
      {
        libremidi_handle_error(configuration, "error sending sysex message");
        // Still try to unprepare
      }
    }

    if (loader.UnprepareLongData)
    {
      loader.UnprepareLongData(&sysex, sizeof(MIDIHDR));
    }

    return stdx::error{};
  }

  std::vector<char> buffer_;
};

}
