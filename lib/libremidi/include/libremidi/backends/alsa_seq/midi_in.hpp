#pragma once
#include <libremidi/backends/alsa_seq/config.hpp>
#include <libremidi/backends/alsa_seq/helpers.hpp>
#include <libremidi/backends/linux/helpers.hpp>
#include <libremidi/detail/midi_in.hpp>
#include <libremidi/detail/midi_stream_decoder.hpp>

NAMESPACE_LIBREMIDI::alsa_seq
{
struct dummy_processing
{
  explicit dummy_processing(auto&&...) { }
};

template <typename ConfigurationImpl>
using midi_in_base
    = std::conditional_t<ConfigurationImpl::midi_version == 1, midi1::in_api, midi2::in_api>;
template <typename ConfigurationImpl>
using midi_in_processing = std::conditional_t<
    ConfigurationImpl::midi_version == 1, midi1::input_state_machine, midi2::input_state_machine>;

template <typename ConfigurationBase, typename ConfigurationImpl>
class midi_in_impl
    : public midi_in_base<ConfigurationImpl>
    , protected alsa_data
    , public error_handler
{
public:
  struct
      : ConfigurationBase
      , ConfigurationImpl
  {
  } configuration;
  midi_in_processing<ConfigurationImpl> m_processing{this->configuration};

  bool require_timestamps() const noexcept
  {
    switch (configuration.timestamps)
    {
      case timestamp_mode::NoTimestamp:
      case timestamp_mode::SystemMonotonic:
      case timestamp_mode::AudioFrame:
        return false;
      case timestamp_mode::Absolute:
      case timestamp_mode::Relative:
      case timestamp_mode::Custom:
        return true;
    }
    return true;
  }

  explicit midi_in_impl(ConfigurationBase&& conf, ConfigurationImpl&& apiconf)
      : midi_in_base<ConfigurationImpl>{}
      , configuration{std::move(conf), std::move(apiconf)}
  {
    if (init_client(configuration) < 0)
    {
      libremidi_handle_error(
          this->configuration,
          "error creating ALSA sequencer client "
          "object.");
      return;
    }

    // Create the input queue
    if (require_timestamps())
    {
      this->queue_id = snd.seq.alloc_queue(seq);
      // Set arbitrary tempo (mm=100) and resolution (240)
      snd_seq_queue_tempo_t* qtempo{};
      snd_seq_queue_tempo_alloca(&qtempo);
      snd.seq.queue_tempo_set_tempo(qtempo, 600000);
      snd.seq.queue_tempo_set_ppq(qtempo, 240);
      snd.seq.set_queue_tempo(this->seq, this->queue_id, qtempo);
      snd.seq.drain_output(this->seq);
    }

    // Create the event -> midi encoder
    {
      int result = snd.midi.event_new(0, &coder);
      if (result < 0)
      {
        libremidi_handle_error(this->configuration, "error during snd_midi_event_new.");
        return;
      }
      snd.midi.event_init(coder);
      snd.midi.event_no_status(coder, 1);
    }
  }

  ~midi_in_impl() override
  {
    // Cleanup.
    if (this->vport >= 0)
      snd.seq.delete_port(this->seq, this->vport);

    if (require_timestamps())
      snd.seq.free_queue(this->seq, this->queue_id);

    snd.midi.event_free(coder);

    // Close if we do not have an user-provided client object
    if (!configuration.context)
      snd.seq.close(this->seq);
  }

  libremidi::API get_current_api() const noexcept override
  {
    if constexpr (ConfigurationImpl::midi_version == 1)
      return libremidi::API::ALSA_SEQ;
    else
      return libremidi::API::ALSA_SEQ_UMP;
  }

  [[nodiscard]] int create_port(std::string_view portName)
  {
    return alsa_data::create_port(
        *this, portName, SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE,
        SND_SEQ_PORT_TYPE_MIDI_GENERIC | SND_SEQ_PORT_TYPE_APPLICATION,
        require_timestamps() ? std::optional<int>{this->queue_id} : std::nullopt);
  }

  void start_queue()
  {
    if (require_timestamps())
    {
      snd.seq.control_queue(this->seq, this->queue_id, SND_SEQ_EVENT_START, 0, nullptr);
      this->queue_creation_time = std::chrono::steady_clock::now();
      snd.seq.drain_output(this->seq);
    }
  }

  void stop_queue()
  {
    if (require_timestamps())
    {
      snd.seq.control_queue(this->seq, this->queue_id, SND_SEQ_EVENT_STOP, 0, nullptr);
      snd.seq.drain_output(this->seq);
    }
  }

  int connect_port(snd_seq_addr_t sender)
  {
    snd_seq_addr_t receiver{};
    receiver.client = snd.seq.client_id(this->seq);
    receiver.port = this->vport;

    return create_connection(*this, sender, receiver, false);
  }

  std::optional<snd_seq_addr_t> to_address(const port_information& p)
  {
    return alsa_data::get_port_info(p);
  }

  int init_port(std::optional<snd_seq_addr_t> source, std::string_view portName)
  {
    this->close_port();

    if (!source)
      return -1;

    if (int ret = create_port(portName); ret < 0)
    {
      libremidi_handle_error(configuration, "ALSA error creating port.");
      return ret;
    }

    if (int ret = connect_port(*source); ret < 0)
    {
      libremidi_handle_error(configuration, "ALSA error making port connection.");
      return ret;
    }

    start_queue();

    return 0;
  }

  int init_virtual_port(std::string_view portName)
  {
    this->close_port();

    if (int ret = create_port(portName); ret < 0)
      return ret;

    start_queue();
    return 0;
  }

  stdx::error close_port() override
  {
    unsubscribe();
    stop_queue();
    return stdx::error{};
  }

  stdx::error set_port_name(std::string_view portName) override
  {
    return alsa_data::set_port_name(portName);
  }

  timestamp absolute_timestamp() const noexcept override
  {
    return std::chrono::duration_cast<std::chrono::nanoseconds>(
               std::chrono::steady_clock::now() - this->queue_creation_time)
        .count();
  }

  int64_t process_event(const snd_seq_event_t& ev)
  {
    if constexpr (ConfigurationImpl::midi_version == 1)
    {
      switch (ev.type)
      {
        case SND_SEQ_EVENT_PORT_SUBSCRIBED:
        case SND_SEQ_EVENT_PORT_UNSUBSCRIBED:
          return 0;
        case SND_SEQ_EVENT_SYSEX: {
          if (configuration.ignore_sysex)
            return 0;
          else if (ev.data.ext.len > decoding_buffer.size())
            decoding_buffer.resize(ev.data.ext.len);
          break;
        }
      }

      static constexpr timestamp_backend_info timestamp_info{
          .has_absolute_timestamps = true,
          .absolute_is_monotonic = false,
          .has_samples = false,
      };

      const auto to_ns = [ts = ev.time.time] {
        return static_cast<int64_t>(ts.tv_sec) * 1'000'000'000 + static_cast<int64_t>(ts.tv_nsec);
      };
      auto buf = decoding_buffer.data();
      auto buf_space = decoding_buffer.size();

      // FIXME according to the doc snd_midi_event_decode can apparently return multiple events????
      const auto avail = snd.midi.event_decode(coder, buf, buf_space, &ev);
      if (avail > 0)
      {
        m_processing.on_bytes(
            {buf, buf + avail}, m_processing.template timestamp<timestamp_info>(to_ns, 0));
        return 0;
      }
      else
      {
        return avail;
      }
    }
    return 0;
  }

  int64_t process_events()
  {
    if constexpr (ConfigurationImpl::midi_version == 1)
    {
      snd_seq_event_t* ev{};
      event_handle handle{snd};
      int64_t result = 0;
      if ((result = snd.seq.event_input(seq, &ev)) > 0)
      {
        handle.reset(ev);
        if (auto err = process_event(*ev); err < 0)
        {
          return err;
        }
      }
      return result;
    }
    else
    {
      return 0;
    }
  }

#if __has_include(<alsa/ump.h>)
  int process_ump_event(const snd_seq_ump_event_t& ev)
  {
    // Filter the message types before any decoding
    switch (ev.type)
    {
      case SND_SEQ_EVENT_PORT_SUBSCRIBED:
      case SND_SEQ_EVENT_PORT_UNSUBSCRIBED:
        return 0;

      case SND_SEQ_EVENT_QFRAME: // MIDI time code
      case SND_SEQ_EVENT_TICK:   // 0xF9 ... MIDI timing tick
      case SND_SEQ_EVENT_CLOCK:  // 0xF8 ... MIDI timing (clock) tick
        if (configuration.ignore_timing)
          return 0;
        break;

      case SND_SEQ_EVENT_SENSING: // Active sensing
        if (configuration.ignore_sensing)
          return 0;
        break;

      case SND_SEQ_EVENT_SYSEX: {
        if (configuration.ignore_sysex)
          return 0;
        break;
      }
    }

    static constexpr timestamp_backend_info timestamp_info{
        .has_absolute_timestamps = true,
        .absolute_is_monotonic = false,
        .has_samples = false,
    };
    const auto to_ns = [ts = ev.time.time] {
      return static_cast<int64_t>(ts.tv_sec) * 1'000'000'000 + static_cast<int64_t>(ts.tv_nsec);
    };

    m_processing.on_bytes_multi(
        {ev.ump, ev.ump + 4}, m_processing.template timestamp<timestamp_info>(to_ns, 0));
    return 0;
  }

  int process_ump_events()
  {
    snd_seq_ump_event_t* ev{};
    event_handle handle{snd};
    int result = 0;
    while ((result = snd.seq.ump.event_input(seq, &ev)) > 0)
    {
      handle.reset((snd_seq_event_t*)ev);
      if (int err = process_ump_event(*ev); err < 0)
        return err;
    }
    return result;
  }
#endif

  int queue_id{}; // an input queue is needed to get timestamped events

  // Only needed for midi 1
  std::vector<unsigned char> decoding_buffer = std::vector<unsigned char>(4096);
  std::chrono::steady_clock::time_point queue_creation_time;
};

template <typename ConfigurationBase, typename ConfigurationImpl>
class midi_in_alsa_threaded : public midi_in_impl<ConfigurationBase, ConfigurationImpl>
{
public:
  midi_in_alsa_threaded(ConfigurationBase&& conf, ConfigurationImpl&& apiconf)
      : midi_in_impl<ConfigurationBase, ConfigurationImpl>{std::move(conf), std::move(apiconf)}
  {
    if (this->m_termination_event < 0)
    {
      this->libremidi_handle_error(this->configuration, "error creating eventfd.");
      return;
    }

    this->client_open_ = stdx::error{};
  }

  ~midi_in_alsa_threaded()
  {
    midi_in_alsa_threaded::close_port();
    this->client_open_ = std::errc::not_connected;
  }

private:
  stdx::error open_port(const input_port& pt, std::string_view local_port_name) override
  {
    if (int err = this->init_port(this->to_address(pt), local_port_name); err < 0)
      return from_errc(err);

    return this->start_thread();
  }

  stdx::error open_virtual_port(std::string_view portName) override
  {
    if (int err = this->init_virtual_port(portName); err < 0)
      return from_errc(err);

    return this->start_thread();
  }

  stdx::error close_port() override
  {
    // FIXME shouldn't we always close the thread
    auto err = midi_in_impl<ConfigurationBase, ConfigurationImpl>::close_port();

    stop_thread();

    return err;
  }

  [[nodiscard]] stdx::error start_thread()
  {
    try
    {
      this->m_thread = std::thread([this] { thread_handler(); });
      return stdx::error{};
    }
    catch (const std::system_error& e)
    {
      using namespace std::literals;
      this->unsubscribe();

      this->libremidi_handle_error(
          this->configuration, "error starting MIDI input thread: "s + e.what());
      return e.code();
    }
  }

  stdx::error stop_thread()
  {
    m_termination_event.notify();

    if (this->m_thread.joinable())
      this->m_thread.join();

    m_termination_event.consume();
    return stdx::error{};
  }

  void thread_handler()
  {
    int poll_fd_count = alsa_data::snd.seq.poll_descriptors_count(this->seq, POLLIN) + 1;
    auto poll_fds = (struct pollfd*)alloca(poll_fd_count * sizeof(struct pollfd));
    poll_fds[0] = this->m_termination_event;
    alsa_data::snd.seq.poll_descriptors(this->seq, poll_fds + 1, poll_fd_count - 1, POLLIN);

    const auto period
        = std::chrono::duration_cast<std::chrono::milliseconds>(this->configuration.poll_period)
              .count();
    for (;;)
    {
      if (alsa_data::snd.seq.event_input_pending(this->seq, 1) == 0)
      {
        // No data pending
        if (poll(poll_fds, poll_fd_count, static_cast<int32_t>(period)) >= 0)
        {
          // We got our stop-thread signal
          if (m_termination_event.ready(poll_fds[0]))
          {
            break;
          }
        }
        continue;
      }

      int64_t res{};
      if constexpr (ConfigurationImpl::midi_version == 1)
      {
        res = this->process_events();
      }
#if __has_include(<alsa/ump.h>)
      else if constexpr (ConfigurationImpl::midi_version == 2)
      {
        res = this->process_ump_events();
      }
#endif

      if (res < 0)
        LIBREMIDI_LOG("MIDI input error: ", this->snd.strerror(res));
    }
  }

  std::thread m_thread{};
  eventfd_notifier m_termination_event{};
};

template <typename ConfigurationBase, typename ConfigurationImpl>
class midi_in_alsa_manual : public midi_in_impl<ConfigurationBase, ConfigurationImpl>
{
public:
  midi_in_alsa_manual(ConfigurationBase&& conf, ConfigurationImpl&& apiconf)
      : midi_in_impl<ConfigurationBase, ConfigurationImpl>{std::move(conf), std::move(apiconf)}
  {
    assert(this->configuration.manual_poll);
    assert(this->configuration.stop_poll);
  }

  [[nodiscard]] int init_callback()
  {
    using poll_params = typename ConfigurationImpl::poll_parameters_type;
    this->configuration.manual_poll(
        poll_params{.addr = this->vaddr, .callback = [this](const auto& ev) {
      if constexpr (ConfigurationImpl::midi_version == 1)
        return this->process_event(ev);
#if __has_include(<alsa/ump.h>)
      else
        return this->process_ump_event(ev);
#endif
    }});
    return 0;
  }

  ~midi_in_alsa_manual() { midi_in_alsa_manual::close_port(); }

  stdx::error open_port(const input_port& pt, std::string_view local_port_name) override
  {
    if (int err = this->init_port(this->to_address(pt), local_port_name); err < 0)
      return from_errc(err);

    if (int err = init_callback(); err < 0)
      return from_errc(err);

    return stdx::error{};
  }

  stdx::error open_virtual_port(std::string_view name) override
  {
    if (int err = this->init_virtual_port(name); err < 0)
      return from_errc(err);

    if (int err = init_callback(); err < 0)
      return from_errc(err);

    return stdx::error{};
  }

  stdx::error close_port() override
  {
    this->configuration.stop_poll(this->vaddr);

    return midi_in_impl<ConfigurationBase, ConfigurationImpl>::close_port();
  }
};
}

NAMESPACE_LIBREMIDI
{
template <>
inline std::unique_ptr<midi_in_api>
make<alsa_seq::midi_in_impl<libremidi::input_configuration, alsa_seq::input_configuration>>(
    libremidi::input_configuration&& conf, libremidi::alsa_seq::input_configuration&& api)
{
  if (api.manual_poll)
    return std::make_unique<alsa_seq::midi_in_alsa_manual<
        libremidi::input_configuration, alsa_seq::input_configuration>>(
        std::move(conf), std::move(api));
  else
    return std::make_unique<alsa_seq::midi_in_alsa_threaded<
        libremidi::input_configuration, alsa_seq::input_configuration>>(
        std::move(conf), std::move(api));
}
}
