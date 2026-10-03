#if defined(__EMSCRIPTEN__)
  #include <libremidi/backends/emscripten/midi_access.hpp>
  #include <libremidi/backends/emscripten/midi_out.hpp>

NAMESPACE_LIBREMIDI
{
LIBREMIDI_INLINE midi_out_emscripten::midi_out_emscripten(
    output_configuration&& conf, emscripten_output_configuration&& apiconf)
    : configuration{std::move(conf), std::move(apiconf)}
{
  client_open_ = stdx::error{};
}

LIBREMIDI_INLINE midi_out_emscripten::~midi_out_emscripten()
{
  // Close a connection if it exists.
  midi_out_emscripten::close_port();
  client_open_ = std::errc::not_connected;
}

LIBREMIDI_INLINE libremidi::API midi_out_emscripten::get_current_api() const noexcept
{
  return libremidi::API::WEBMIDI;
}

LIBREMIDI_INLINE stdx::error midi_out_emscripten::open_port(int portNumber, std::string_view)
{
  auto& midi = webmidi_helpers::midi_access_emscripten::instance();

  if (portNumber >= midi.output_count())
  {
    libremidi_handle_error(this->configuration, "no MIDI output sources found.");
    return std::errc::invalid_argument;
  }

  m_portNumber = portNumber;
  return stdx::error{};
}

LIBREMIDI_INLINE stdx::error
midi_out_emscripten::open_port(const output_port& p, std::string_view nm)
{
  return open_port(p.port, nm);
}

LIBREMIDI_INLINE stdx::error midi_out_emscripten::close_port()
{
  return stdx::error{};
}

LIBREMIDI_INLINE stdx::error
midi_out_emscripten::send_message(const unsigned char* message, size_t size)
{
  if (m_portNumber < 0)
    libremidi_handle_error(
        this->configuration,
        "trying to send a message without an open "
        "port.");

  webmidi_helpers::midi_access_emscripten::instance().send_message(
      m_portNumber, reinterpret_cast<const char*>(message), size);
  return stdx::error{};
}
}
#endif
