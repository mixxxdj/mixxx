#pragma once
#include <libremidi/backends/alsa_raw/config.hpp>
#include <libremidi/backends/alsa_raw/helpers.hpp>
#include <libremidi/backends/linux/helpers.hpp>
#include <libremidi/detail/midi_in.hpp>
#include <libremidi/detail/midi_stream_decoder.hpp>

#include <alsa/asoundlib.h>

#include <chrono>
#include <thread>

NAMESPACE_LIBREMIDI::alsa_raw
{
class midi_in_impl
    : public midi1::in_api
    , public error_handler
{
public:
  struct
      : input_configuration
      , alsa_raw_input_configuration
  {
  } configuration;

  const libasound& snd = libasound::instance();

  explicit midi_in_impl(input_configuration&& conf, alsa_raw_input_configuration&& apiconf)
      : configuration{std::move(conf), std::move(apiconf)}
  {
    fds_.reserve(4);
  }

  ~midi_in_impl() override { }

  libremidi::API get_current_api() const noexcept override { return libremidi::API::ALSA_RAW; }

  // Must be a string such as: "hw:2,4,1"
  [[nodiscard]] stdx::error do_init_port(const char* portname)
  {
    constexpr int mode = SND_RAWMIDI_NONBLOCK;
    if (const int err = snd.rawmidi.open(&midiport_, nullptr, portname, mode); err < 0)
    {
      libremidi_handle_error(this->configuration, "cannot open device.");
      return from_errc(err);
    }

    snd_rawmidi_params_t* params{};
    snd_rawmidi_params_alloca(&params);

    if (const int err = snd.rawmidi.params_current(midiport_, params); err < 0)
      return from_errc(err);
    if (const int err = snd.rawmidi.params_set_no_active_sensing(midiport_, params, 1); err < 0)
      return from_errc(err);
#if LIBREMIDI_ALSA_HAS_RAWMIDI_TREAD
    if (configuration.timestamps == timestamp_mode::NoTimestamp)
    {
      if (const int err
          = snd.rawmidi.params_set_read_mode(midiport_, params, SND_RAWMIDI_READ_STANDARD);
          err < 0)
        return from_errc(err);
      if (const int err
          = snd.rawmidi.params_set_clock_type(midiport_, params, SND_RAWMIDI_CLOCK_NONE);
          err < 0)
        return from_errc(err);
    }
    else
    {
      if (const int err
          = snd.rawmidi.params_set_read_mode(midiport_, params, SND_RAWMIDI_READ_TSTAMP);
          err < 0)
        return from_errc(err);
      if (const int err
          = snd.rawmidi.params_set_clock_type(midiport_, params, SND_RAWMIDI_CLOCK_MONOTONIC);
          err < 0)
        return from_errc(err);
    }
#endif

    if (const int err = snd.rawmidi.params(midiport_, params); err < 0)
      return from_errc(err);

    return init_pollfd();
  }

  [[nodiscard]] stdx::error init_port(const port_information& p)
  {
    return do_init_port(raw_from_port_handle(p.port).to_string().c_str());
  }

  [[nodiscard]] stdx::error init_pollfd()
  {
    const int num_fds = snd.rawmidi.poll_descriptors_count(this->midiport_);

    this->fds_.clear();
    this->fds_.resize(num_fds);

    int ret = snd.rawmidi.poll_descriptors(this->midiport_, fds_.data(), num_fds);
    if (ret < 0)
      return from_errc(ret);
    return stdx::error{};
  }

  ssize_t do_read_events(auto parse_func, std::span<pollfd> fds)
  {
    // Read events
    if (fds.empty())
    {
      return (this->*parse_func)();
    }
    else
    {
      unsigned short res{};
      const int err = snd.rawmidi.poll_descriptors_revents(
          this->midiport_, fds.data(), static_cast<unsigned int>(fds.size()), &res);
      if (err < 0)
        return err;

      // Did we encounter an error during polling
      if (res & (POLLERR | POLLHUP))
        return -EIO;

      // Is there data to read
      if (res & POLLIN)
        return (this->*parse_func)();
    }

    return 0;
  }

  ssize_t read_input_buffer()
  {
    static const constexpr int nbytes = 1024;
    static constexpr timestamp_backend_info timestamp_info{
        .has_absolute_timestamps = false,
        .absolute_is_monotonic = false,
        .has_samples = false,
    };

    unsigned char bytes[nbytes];

    ssize_t err = 0;
    // err is the amount of bytes read
    while ((err = snd.rawmidi.read(this->midiport_, bytes, nbytes)) > 0)
    {
      const auto to_ns = [this] { return absolute_timestamp(); };
      m_processing.on_bytes(
          {bytes, bytes + err}, m_processing.timestamp<timestamp_info>(to_ns, 0));
    }
    return err;
  }

#if LIBREMIDI_ALSA_HAS_RAWMIDI_TREAD
  ssize_t read_input_buffer_with_timestamps()
  {
    static constexpr int nbytes = 1024;
    static constexpr timestamp_backend_info timestamp_info{
        .has_absolute_timestamps = true,
        .absolute_is_monotonic = true,
        .has_samples = false,
    };

    unsigned char bytes[nbytes];
    struct timespec ts;

    ssize_t err = 0;
    // err is the amount of bytes read
    while ((err = snd.rawmidi.tread(this->midiport_, &ts, bytes, nbytes)) > 0)
    {
      const auto to_ns = [ts] {
        return static_cast<int64_t>(ts.tv_sec) * 1'000'000'000 + static_cast<int64_t>(ts.tv_nsec);
      };
      m_processing.on_bytes(
          {bytes, bytes + err}, m_processing.timestamp<timestamp_info>(to_ns, 0));
    }
    return err;
  }
#else
  ssize_t read_input_buffer_with_timestamps() { return read_input_buffer(); }
#endif

  stdx::error close_port() override
  {
    if (midiport_)
      snd.rawmidi.close(midiport_);
    midiport_ = nullptr;
    return stdx::error{};
  }

  timestamp absolute_timestamp() const noexcept final override { return system_ns(); }

  snd_rawmidi_t* midiport_{};
  std::vector<pollfd> fds_;
  midi1::input_state_machine m_processing{this->configuration};
};

class midi_in_alsa_raw_threaded : public midi_in_impl
{
public:
  midi_in_alsa_raw_threaded(input_configuration&& conf, alsa_raw_input_configuration&& apiconf)
      : midi_in_impl{std::move(conf), std::move(apiconf)}
  {
    if (this->m_termination_event < 0)
    {
      libremidi_handle_error(this->configuration, "error creating eventfd.");
      return;
    }

    client_open_ = stdx::error{};
  }

  ~midi_in_alsa_raw_threaded() override
  {
    // Close a connection if it exists.
    this->midi_in_alsa_raw_threaded::close_port();

    client_open_ = std::errc::not_connected;
  }

private:
  void run_thread(auto parse_func)
  {
    fds_.push_back(this->m_termination_event);
    const auto period
        = std::chrono::duration_cast<std::chrono::milliseconds>(this->configuration.poll_period)
              .count();

    for (;;)
    {
      // Poll
      ssize_t err = poll(fds_.data(), fds_.size(), static_cast<int32_t>(period));
      if (err == -EAGAIN)
        continue;
      else if (err < 0)
        return;
      else if (m_termination_event.ready(fds_.back()))
        break;

      err = do_read_events(parse_func, {fds_.data(), fds_.size() - 1});
      if (err == -EAGAIN)
        continue;
      else if (err < 0)
        return;
    }
  }

  [[nodiscard]] stdx::error start_thread()
  {
    try
    {
      if (configuration.timestamps == timestamp_mode::NoTimestamp)
      {
        this->m_thread = std::thread{[this] { run_thread(&midi_in_impl::read_input_buffer); }};
      }
      else
      {
        this->m_thread = std::thread{
            [this] { run_thread(&midi_in_impl::read_input_buffer_with_timestamps); }};
      }
      return stdx::error{};
    }
    catch (const std::system_error& e)
    {
      using namespace std::literals;

      libremidi_handle_error(
          this->configuration, "error starting MIDI input thread: "s + e.what());
      return e.code();
    }
    return stdx::error{};
  }

  stdx::error open_port(const input_port& port, std::string_view /*name*/) override
  {
    if (auto err = midi_in_impl::init_port(port); err != stdx::error{})
      return err;
    if (auto err = start_thread(); err != stdx::error{})
      return err;
    return stdx::error{};
  }

  stdx::error close_port() override
  {
    m_termination_event.notify();
    if (m_thread.joinable())
      m_thread.join();
    m_termination_event.consume(); // Reset to zero

    return midi_in_impl::close_port();
  }

  std::thread m_thread;
  eventfd_notifier m_termination_event{};
};

class midi_in_alsa_raw_manual : public midi_in_impl
{
public:
  midi_in_alsa_raw_manual(input_configuration&& conf, alsa_raw_input_configuration&& apiconf)
      : midi_in_impl{std::move(conf), std::move(apiconf)}
  {
    client_open_ = stdx::error{};
  }

  ~midi_in_alsa_raw_manual()
  {
    // Close a connection if it exists.
    this->close_port();

    client_open_ = std::errc::not_connected;
  }

private:
  void send_poll_callback()
  {
    if (configuration.timestamps == timestamp_mode::NoTimestamp)
    {
      configuration.manual_poll(
          manual_poll_parameters{
              .fds = {this->fds_.data(), this->fds_.size()},
              .callback = [this](std::span<pollfd> fds) {
        return do_read_events(&midi_in_impl::read_input_buffer, fds);
      }});
    }
    else
    {
      configuration.manual_poll(
          manual_poll_parameters{
              .fds = {this->fds_.data(), this->fds_.size()},
              .callback = [this](std::span<pollfd> fds) {
        return do_read_events(&midi_in_impl::read_input_buffer_with_timestamps, fds);
      }});
    }
  }

  stdx::error open_port(const input_port& p, std::string_view /*name*/) override
  {
    if (auto err = midi_in_impl::init_port(p); err != stdx::error{})
      return err;
    send_poll_callback();
    return stdx::error{};
  }
};
}

NAMESPACE_LIBREMIDI
{
template <>
inline std::unique_ptr<midi_in_api> make<alsa_raw::midi_in_impl>(
    libremidi::input_configuration&& conf, libremidi::alsa_raw_input_configuration&& api)
{
  if (api.manual_poll)
    return std::make_unique<alsa_raw::midi_in_alsa_raw_manual>(std::move(conf), std::move(api));
  else
    return std::make_unique<alsa_raw::midi_in_alsa_raw_threaded>(std::move(conf), std::move(api));
}
}
