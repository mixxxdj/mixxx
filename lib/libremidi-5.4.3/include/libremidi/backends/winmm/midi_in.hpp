#pragma once
#include <libremidi/backends/winmm/config.hpp>
#include <libremidi/backends/winmm/helpers.hpp>
#include <libremidi/backends/winmm/observer.hpp>
#include <libremidi/detail/midi_in.hpp>
#include <libremidi/detail/midi_stream_decoder.hpp>

NAMESPACE_LIBREMIDI
{

class midi_in_winmm final
    : public midi1::in_api
    , public error_handler
{
public:
  struct
      : input_configuration
      , winmm_input_configuration
  {
  } configuration;

  explicit midi_in_winmm(input_configuration&& conf, winmm_input_configuration&& apiconf)
      : configuration{std::move(conf), std::move(apiconf)}
  {
    if (!InitializeCriticalSectionAndSpinCount(&(this->_mutex), 0x00000400))
    {
      libremidi_handle_error(configuration, "InitializeCriticalSectionAndSpinCount failed.");

      this->client_open_ = std::errc::too_many_files_open;
      return;
    }

    this->client_open_ = stdx::error{};
  }

  ~midi_in_winmm() override
  {
    // Close a connection if it exists.
    midi_in_winmm::close_port();

    if (this->client_open_ == stdx::error{})
      DeleteCriticalSection(&(this->_mutex));
  }

  libremidi::API get_current_api() const noexcept override { return libremidi::API::WINDOWS_MM; }

  stdx::error do_open(std::size_t portNumber)
  {
    MMRESULT result = midiInOpen(
        &this->inHandle, portNumber, std::bit_cast<DWORD_PTR>(&midiInputCallback),
        std::bit_cast<DWORD_PTR>(this), CALLBACK_FUNCTION);
    if (result != MMSYSERR_NOERROR)
    {
      libremidi_handle_error(configuration, "error creating Windows MM MIDI input port.");
      return from_mmerr(result);
    }

    // Allocate and init the sysex buffers.
    const auto bufferCount = static_cast<std::size_t>(configuration.sysex_buffer_count);
    this->sysexBuffer.resize(bufferCount);
    for (std::size_t i = 0; i < bufferCount; ++i)
    {
      this->sysexBuffer[i] = new MIDIHDR;
      this->sysexBuffer[i]->lpData = new char[configuration.sysex_buffer_size];
      this->sysexBuffer[i]->dwBufferLength = static_cast<DWORD>(configuration.sysex_buffer_size);
      this->sysexBuffer[i]->dwUser = i; // We use the dwUser parameter as buffer indicator
      this->sysexBuffer[i]->dwFlags = 0;

      result = midiInPrepareHeader(this->inHandle, this->sysexBuffer[i], sizeof(MIDIHDR));
      if (result != MMSYSERR_NOERROR)
      {
        midiInClose(this->inHandle);
        this->inHandle = nullptr;
        libremidi_handle_error(
            configuration,
            "error starting Windows MM MIDI input port "
            "(PrepareHeader).");
        return from_mmerr(result);
      }

      // Register the buffer.
      result = midiInAddBuffer(this->inHandle, this->sysexBuffer[i], sizeof(MIDIHDR));
      if (result != MMSYSERR_NOERROR)
      {
        midiInClose(this->inHandle);
        this->inHandle = nullptr;
        libremidi_handle_error(
            configuration,
            "error starting Windows MM MIDI input port "
            "(AddBuffer).");
        return from_mmerr(result);
      }
    }

    result = midiInStart(this->inHandle);
    midi_start_timestamp = std::chrono::steady_clock::now();
    if (result != MMSYSERR_NOERROR)
    {
      midiInClose(this->inHandle);
      this->inHandle = nullptr;
      libremidi_handle_error(configuration, "error starting Windows MM MIDI input port.");
      return from_mmerr(result);
    }

    return stdx::error{};
  }

  stdx::error open_port(const input_port& p, std::string_view) override
  {
    observer_winmm obs{{}, winmm_observer_configuration{}};
    auto ports = obs.get_input_ports();

    // First check with the display name, e.g. MIDI KEYBOARD 2 will match MIDI KEYBOARD 2
    for (auto& port : ports)
    {
      if (p.display_name == port.display_name)
        return do_open(port.port);
    }
    // If nothing is found, try to check with the raw name
    for (auto& port : ports)
    {
      if (p.port_name == port.port_name)
        return do_open(port.port);
    }
    libremidi_handle_error(configuration, "port not found: " + p.port_name);
    return std::errc::invalid_argument;
  }

  stdx::error close_port() override
  {
    if (connected_)
    {
      EnterCriticalSection(&(this->_mutex));
      midiInReset(this->inHandle);
      midiInStop(this->inHandle);

      for (std::size_t i = 0; i < static_cast<std::size_t>(configuration.sysex_buffer_count); ++i)
      {
        MMRESULT res{};

        int wait_count = 5;
        while (
            ((res = midiInUnprepareHeader(this->inHandle, this->sysexBuffer[i], sizeof(MIDIHDR)))
             == MIDIERR_STILLPLAYING)
            && wait_count-- >= 0)
        {
          Sleep(1);
        }

        if (res != MMSYSERR_NOERROR)
        {
          libremidi_handle_warning(
              configuration,
              "error closing Windows MM MIDI input "
              "port (midiInUnprepareHeader).");
          continue;
        }
        else
        {
          delete[] this->sysexBuffer[i]->lpData;
          delete[] this->sysexBuffer[i];
        }
      }

      midiInClose(this->inHandle);
      this->inHandle = nullptr;
      LeaveCriticalSection(&(this->_mutex));
    }
    return stdx::error{};
  }

private:
  timestamp absolute_timestamp() const noexcept override
  {
    return std::chrono::duration_cast<std::chrono::nanoseconds>(
               std::chrono::steady_clock::now() - midi_start_timestamp)
        .count();
  }

  static constexpr int bytes_for_message(uint8_t status)
  {
    if (status < 0xC0)
      return 3;
    else if (status < 0xE0)
      return 2;
    else if (status < 0xF0)
      return 3;
    else if (status == 0xF1)
      return 2;
    else if (status == 0xF2)
      return 3;
    else if (status == 0xF3)
      return 2;
    else if (status == 0xF8)
      return 1;
    else if (status == 0xFE)
      return 1;
    else
      return 0;
  }

  static void CALLBACK midiInputCallback(
      HMIDIIN /*hmin*/, UINT inputStatus, DWORD_PTR instancePtr, DWORD_PTR midiMessage,
      DWORD_PTR timestamp)
  {
    if (inputStatus != MIM_DATA && inputStatus != MIM_LONGDATA && inputStatus != MIM_LONGERROR)
      return;

    auto& self = *reinterpret_cast<midi_in_winmm*>(instancePtr);
    static constexpr timestamp_backend_info timestamp_info{
        .has_absolute_timestamps = true,
        .absolute_is_monotonic = false,
        .has_samples = false,
    };

    const auto to_ns = [timestamp] { return timestamp * 1'000'000; };

    if (inputStatus == MIM_DATA)
    {
      // Channel or system message
      uint8_t message[sizeof(DWORD_PTR)];
      memcpy(message, &midiMessage, sizeof(DWORD_PTR));

      // Make sure the first byte is a status byte.
      if (message[0] & 0x80)
      {
        self.m_processing.on_bytes(
            {message, message + bytes_for_message(message[0])},
            self.m_processing.timestamp<timestamp_info>(to_ns, 0));
      }
    }
    else
    {
      // Sysex message ( MIM_LONGDATA or MIM_LONGERROR )
      const auto* sysex = reinterpret_cast<MIDIHDR*>(midiMessage);
      if (inputStatus == MIM_LONGERROR)
      {
        self.m_processing.reset();
      }
      else if (!self.configuration.ignore_sysex)
      {
        if (sysex->dwBytesRecorded > 0)
        {
          const auto sysex_bytes = reinterpret_cast<uint8_t*>(sysex->lpData);

          self.m_processing.on_bytes(
              {sysex_bytes, sysex_bytes + sysex->dwBytesRecorded},
              self.m_processing.timestamp<timestamp_info>(to_ns, 0));
        }
      }

      // The WinMM API requires that the sysex buffer be requeued after
      // input of each sysex message.  Even if we are ignoring sysex
      // messages, we still need to requeue the buffer in case the user
      // decides to not ignore sysex messages in the future.  However,
      // it seems that WinMM calls this function with an empty sysex
      // buffer when an application closes and in this case, we should
      // avoid requeueing it, else the computer suddenly reboots after
      // one or two minutes.
      if (self.sysexBuffer[sysex->dwUser]->dwBytesRecorded > 0)
      {
        EnterCriticalSection(&(self._mutex));
        MMRESULT result
            = midiInAddBuffer(self.inHandle, self.sysexBuffer[sysex->dwUser], sizeof(MIDIHDR));
        LeaveCriticalSection(&(self._mutex));
        if (result != MMSYSERR_NOERROR)
        {
          LIBREMIDI_LOG(
              "error sending sysex to "
              "Midi device!!");
        }
      }
    }
  }

  HMIDIIN inHandle; // Handle to Midi Input Device

  std::vector<LPMIDIHDR> sysexBuffer;
  // [Patrice] see
  // https://groups.google.com/forum/#!topic/mididev/6OUjHutMpEo
  CRITICAL_SECTION _mutex;
  std::chrono::steady_clock::time_point midi_start_timestamp;

  midi1::input_state_machine m_processing{this->configuration};
};

}
