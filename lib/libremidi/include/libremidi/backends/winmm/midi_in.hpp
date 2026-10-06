#pragma once
#include <libremidi/backends/winmm/config.hpp>
#include <libremidi/backends/winmm/helpers.hpp>
#include <libremidi/backends/winmm/observer.hpp>
#include <libremidi/detail/midi_in.hpp>
#include <libremidi/detail/midi_stream_decoder.hpp>

#include <atomic>

NAMESPACE_LIBREMIDI
{

class midi_in_winmm final
    : public midi1::in_api
    , public error_handler
{
  // A user callback that throws would otherwise leave the critical section
  // owned, and the destructor would then delete an owned one.
  struct scoped_leave
  {
    explicit scoped_leave(CRITICAL_SECTION& c) noexcept
        : cs{c}
    {
    }
    ~scoped_leave() { LeaveCriticalSection(&cs); }

    scoped_leave(const scoped_leave&) = delete;
    scoped_leave& operator=(const scoped_leave&) = delete;

    CRITICAL_SECTION& cs;
  };

  // Registers the callback with do_close(), which waits for the count to reach
  // zero before it touches anything the callback may be using.
  struct callback_guard
  {
    explicit callback_guard(midi_in_winmm& s) noexcept
        : self{s}
    {
      self.callback_thread.store(GetCurrentThreadId());
      self.in_callback.fetch_add(1);
    }
    ~callback_guard()
    {
      self.in_callback.fetch_sub(1);
      self.callback_thread.store(0);
    }

    callback_guard(const callback_guard&) = delete;
    callback_guard& operator=(const callback_guard&) = delete;

    midi_in_winmm& self;
  };

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
    const auto closed = midi_in_winmm::close_port();

    // A failed close leaves the device holding a callback into this object.
    if (this->client_open_ == stdx::error{} && closed == stdx::error{})
      DeleteCriticalSection(&(this->_mutex));
  }

  libremidi::API get_current_api() const noexcept override { return libremidi::API::WINDOWS_MM; }

  stdx::error do_open(std::size_t portNumber)
  {
    if (this->inHandle)
    {
      // A previous close failed: reopening would overwrite the handle of a
      // device that is still running and leak it for good.
      libremidi_handle_error(configuration, "Windows MM MIDI input port is still open.");
      return std::errc::device_or_resource_busy;
    }

    // Keep callbacks out until the setup is complete.
    this->closing.store(true);

    MMRESULT result = midiInOpen(
        &this->inHandle, portNumber, std::bit_cast<DWORD_PTR>(&midiInputCallback),
        std::bit_cast<DWORD_PTR>(this), CALLBACK_FUNCTION);
    if (result != MMSYSERR_NOERROR)
    {
      this->inHandle = nullptr;
      libremidi_handle_error(configuration, "error creating Windows MM MIDI input port.");
      return from_mmerr(result);
    }

    // Allocate and init the sysex buffers.
    const auto bufferCount = static_cast<std::size_t>(configuration.sysex_buffer_count);
    this->sysexBuffer.assign(bufferCount, nullptr);
    for (std::size_t i = 0; i < bufferCount; ++i)
    {
      // WinMM requires the reserved members to be zeroed.
      auto* const hdr = new MIDIHDR{};
      this->sysexBuffer[i] = hdr;

      hdr->lpData = new char[configuration.sysex_buffer_size];
      hdr->dwBufferLength = static_cast<DWORD>(configuration.sysex_buffer_size);
      hdr->dwUser = i; // We use the dwUser parameter as buffer indicator
      hdr->dwFlags = 0;

      result = midiInPrepareHeader(this->inHandle, hdr, sizeof(MIDIHDR));
      if (result != MMSYSERR_NOERROR)
      {
        libremidi_handle_error(
            configuration,
            "error starting Windows MM MIDI input port "
            "(PrepareHeader).");
        midi_in_winmm::close_port();
        return from_mmerr(result);
      }

      // Register the buffer.
      result = midiInAddBuffer(this->inHandle, hdr, sizeof(MIDIHDR));
      if (result != MMSYSERR_NOERROR)
      {
        libremidi_handle_error(
            configuration,
            "error starting Windows MM MIDI input port "
            "(AddBuffer).");
        midi_in_winmm::close_port();
        return from_mmerr(result);
      }
    }

    // Everything the callback may touch is now in place.
    this->closing.store(false);

    result = midiInStart(this->inHandle);
    midi_start_timestamp = std::chrono::steady_clock::now();
    if (result != MMSYSERR_NOERROR)
    {
      libremidi_handle_error(configuration, "error starting Windows MM MIDI input port.");
      midi_in_winmm::close_port();
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
    // Both midi_in::close_port() and ~midi_in_winmm() end up here, and the port
    // may never have been opened at all - in which case _mutex may not even
    // have been initialized, so this check has to come before the locking.
    if (!this->inHandle)
      return stdx::error{};

    EnterCriticalSection(&(this->_mutex));
    const scoped_leave leave{this->_mutex};
    return do_close();
  }

private:
  // Closes the device and releases the sysex buffers.
  //
  // The buffers are only freed once midiInClose() succeeded: for as long as the
  // device is open, the driver - and midiInputCallback(), which requeues them -
  // can still hand them back to WinMM. Freeing them any earlier lets the driver
  // write into freed memory, which surfaces much later as heap corruption in an
  // unrelated allocation.
  //
  // Must be called with _mutex held.
  stdx::error do_close()
  {
    this->closing.store(true);

    const HMIDIIN handle = this->inHandle;

    if (handle)
    {
      midiInStop(handle);
      midiInReset(handle);
    }

    // Sleep(1) lasts a timer tick, ~15.6ms unless something in the process
    // raised the resolution, so bound the teardown by a deadline rather than by
    // a number of tries.
    const auto deadline = std::chrono::steady_clock::now() + close_timeout;

    // Wait out the callbacks still running: they may be reading the buffers we
    // are about to free. This pairs with the fetch_add in callback_guard, and
    // both sides need sequential consistency - the callback registers before it
    // reads `closing`, so either it is counted here or it sees the store and
    // gives up.
    //
    // Note what this deliberately is not: a lock. Holding one across the user's
    // message callback would invert against any lock the closing thread already
    // holds - which, in a host tearing a device down, is not hypothetical.
    // Waiting can only cost the deadline.
    bool quiesced = true;
    if (this->callback_thread.load() != GetCurrentThreadId())
    {
      while (this->in_callback.load() != 0)
      {
        if (std::chrono::steady_clock::now() >= deadline)
        {
          quiesced = false;
          break;
        }
        SwitchToThread();
      }
    }

    const auto retry_while_playing = [deadline](auto&& fn) {
      for (;;)
      {
        const MMRESULT res = fn();
        if (res != MIDIERR_STILLPLAYING || std::chrono::steady_clock::now() >= deadline)
          return res;
        Sleep(1);
      }
    };

    for (LPMIDIHDR hdr : this->sysexBuffer)
    {
      // do_open() may have failed halfway through preparing the buffers.
      if (!handle || !hdr || !(hdr->dwFlags & MHDR_PREPARED))
        continue;

      const MMRESULT res = retry_while_playing(
          [&] { return midiInUnprepareHeader(handle, hdr, sizeof(MIDIHDR)); });

      if (res != MMSYSERR_NOERROR)
        libremidi_handle_warning(
            configuration,
            "error closing Windows MM MIDI input "
            "port (midiInUnprepareHeader).");
    }

    MMRESULT close_res = MMSYSERR_NOERROR;
    if (handle)
      close_res = retry_while_playing([&] { return midiInClose(handle); });

    if (close_res != MMSYSERR_NOERROR)
    {
      // The device is still open and may keep writing into the buffers. Keep
      // the handle and the buffers so that a later close_port() - the
      // destructor's, at the latest - can retry the whole teardown.
      libremidi_handle_warning(
          configuration,
          "error closing Windows MM MIDI input "
          "port (midiInClose).");
      return from_mmerr(close_res);
    }

    this->inHandle = nullptr;

    if (!quiesced)
    {
      // A callback is wedged somewhere in user code and may still be reading
      // the buffers. The device is closed so no further ones can start, but
      // these cannot be reclaimed.
      libremidi_handle_warning(
          configuration,
          "timed out waiting for the Windows MM MIDI input "
          "callback: leaking the sysex buffers.");
      return std::errc::timed_out;
    }

    for (LPMIDIHDR hdr : this->sysexBuffer)
    {
      if (!hdr)
        continue;
      delete[] hdr->lpData;
      delete hdr;
    }
    this->sysexBuffer.clear();

    return stdx::error{};
  }

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

    switch (status)
    {
      // System common
      case 0xF1: // MIDI time code quarter frame
        return 2;
      case 0xF2: // Song position pointer
        return 3;
      case 0xF3: // Song select
        return 2;
      case 0xF6: // Tune request
        return 1;

      // System real-time
      case 0xF8: // Timing clock
      case 0xFA: // Start
      case 0xFB: // Continue
      case 0xFC: // Stop
      case 0xFE: // Active sensing
      case 0xFF: // Reset
        return 1;

      // 0xF0 and 0xF7 are sysex, which does not come through MIM_DATA; 0xF4,
      // 0xF5, 0xF9 and 0xFD are undefined.
      default:
        return 0;
    }
  }

  static void CALLBACK midiInputCallback(
      HMIDIIN /*hmin*/, UINT inputStatus, DWORD_PTR instancePtr, DWORD_PTR midiMessage,
      DWORD_PTR timestamp)
  {
    if (inputStatus != MIM_DATA && inputStatus != MIM_LONGDATA && inputStatus != MIM_LONGERROR)
      return;

    auto& self = *reinterpret_cast<midi_in_winmm*>(instancePtr);

    // Registering before reading `closing` is what makes do_close() wait for
    // this callback instead of racing it.
    const callback_guard guard{self};
    if (self.closing.load())
      return;

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
      return;
    }

    // Sysex message ( MIM_LONGDATA or MIM_LONGERROR )
    auto* const sysex = reinterpret_cast<MIDIHDR*>(midiMessage);

    // dwUser comes back from the driver: only trust it if it still designates
    // one of our own buffers.
    const auto index = static_cast<std::size_t>(sysex->dwUser);
    if (index >= self.sysexBuffer.size() || self.sysexBuffer[index] != sysex)
      return;

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
    //
    // The closing check is not redundant: on_bytes() above ran user code, which
    // may have closed the port from this very thread and freed the header we
    // are about to requeue.
    if (!self.closing.load() && sysex->dwBytesRecorded > 0)
    {
      if (midiInAddBuffer(self.inHandle, sysex, sizeof(MIDIHDR)) != MMSYSERR_NOERROR)
      {
        LIBREMIDI_LOG(
            "error sending sysex to "
            "Midi device!!");
      }
    }
  }

  // Total budget for a teardown, which runs on whichever thread closes the
  // port - often the UI one.
  static constexpr std::chrono::milliseconds close_timeout{50};

  HMIDIIN inHandle{}; // Handle to Midi Input Device, null when the port is closed

  std::vector<LPMIDIHDR> sysexBuffer;
  // [Patrice] see
  // https://groups.google.com/forum/#!topic/mididev/6OUjHutMpEo
  CRITICAL_SECTION _mutex;

  std::atomic_bool closing{false};
  std::atomic_int in_callback{0};

  // Lets a close requested from inside the callback skip the wait it would
  // otherwise deadlock on.
  std::atomic<DWORD> callback_thread{};

  std::chrono::steady_clock::time_point midi_start_timestamp;

  midi1::input_state_machine m_processing{this->configuration};
};

}
