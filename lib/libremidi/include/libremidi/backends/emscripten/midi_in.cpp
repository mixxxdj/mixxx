#if defined(__EMSCRIPTEN__)
  #include <libremidi/backends/emscripten/midi_access.hpp>
  #include <libremidi/backends/emscripten/midi_in.hpp>
  #include <libremidi/detail/midi_stream_decoder.hpp>

  #include <chrono>

NAMESPACE_LIBREMIDI
{
LIBREMIDI_INLINE midi_in_emscripten::midi_in_emscripten(
    input_configuration&& conf, emscripten_input_configuration&& apiconf)
    : configuration{std::move(conf), std::move(apiconf)}
{
  client_open_ = stdx::error{};
}

LIBREMIDI_INLINE midi_in_emscripten::~midi_in_emscripten()
{
  // Close a connection if it exists.
  midi_in_emscripten::close_port();
  client_open_ = std::errc::not_connected;
}

LIBREMIDI_INLINE libremidi::API midi_in_emscripten::get_current_api() const noexcept
{
  return libremidi::API::WEBMIDI;
}

LIBREMIDI_INLINE stdx::error midi_in_emscripten::open_port(int portNumber, std::string_view)
{
  auto& midi = webmidi_helpers::midi_access_emscripten::instance();

  if (portNumber < 0 || portNumber >= midi.input_count())
  {
    libremidi_handle_error(this->configuration, "no MIDI output sources found.");
    return std::errc::invalid_argument;
  }

  midi.open_input(portNumber, *this);
  m_portNumber = portNumber;
  return stdx::error{};
}

LIBREMIDI_INLINE stdx::error
midi_in_emscripten::open_port(const libremidi::input_port& p, std::string_view nm)
{
  return open_port(p.port, nm);
}

LIBREMIDI_INLINE stdx::error midi_in_emscripten::close_port()
{
  auto& midi = webmidi_helpers::midi_access_emscripten::instance();

  midi.close_input(m_portNumber, *this);

  return stdx::error{};
}

LIBREMIDI_INLINE int64_t midi_in_emscripten::absolute_timestamp() const noexcept
{
  return system_ns();
}

LIBREMIDI_INLINE void
midi_in_emscripten::on_input(double ts, unsigned char* begin, unsigned char* end)
{
  static constexpr timestamp_backend_info timestamp_info{
      .has_absolute_timestamps = true,
      .absolute_is_monotonic = true,
      .has_samples = false,
  };
  const auto to_ns = [=] { return 1e6 * ts; };

  m_processing.on_bytes({begin, end}, m_processing.timestamp<timestamp_info>(to_ns, 0));
}

}
#endif
