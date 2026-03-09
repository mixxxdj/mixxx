#if defined(__EMSCRIPTEN__)
  #include <libremidi/backends/emscripten/midi_access.hpp>
  #include <libremidi/backends/emscripten/observer.hpp>

  #include <cassert>

NAMESPACE_LIBREMIDI
{
LIBREMIDI_INLINE observer_emscripten::observer_emscripten(
    observer_configuration&& conf, emscripten_observer_configuration&& apiconf)
    : configuration{std::move(conf), std::move(apiconf)}
{
  if (!configuration.has_callbacks())
    return;

  auto& webmidi = webmidi_helpers::midi_access_emscripten::instance();
  webmidi.register_observer(*this);

  // Trigger an initial notification
  webmidi.load_current_infos();

  if (configuration.notify_in_constructor)
    update(webmidi.inputs(), webmidi.outputs());
}

LIBREMIDI_INLINE observer_emscripten::~observer_emscripten()
{
  if (!configuration.has_callbacks())
    return;

  webmidi_helpers::midi_access_emscripten::instance().unregister_observer(*this);
}

LIBREMIDI_INLINE
libremidi::API observer_emscripten::get_current_api() const noexcept
{
  return libremidi::API::WEBMIDI;
}

template <bool Input>
static auto to_port_info(int index, const webmidi_helpers::device_information& dev)
    -> std::conditional_t<Input, input_port, output_port>
{
  return {
      {.api = libremidi::API::WEBMIDI,
       .client = 0,
       .port = (uint64_t)index,
       .manufacturer = "",
       .device_name = "",
       .port_name = dev.name,
       .display_name = dev.name}};
}

LIBREMIDI_INLINE
std::vector<libremidi::input_port> observer_emscripten::get_input_ports() const noexcept
{
  std::vector<libremidi::input_port> ret;
  auto& webmidi = webmidi_helpers::midi_access_emscripten::instance();
  webmidi.load_current_infos();

  for (std::size_t i = 0, n = webmidi.input_count(); i < n; i++)
  {
    ret.push_back(to_port_info<true>(i, webmidi.inputs()[i]));
  }

  return ret;
}

LIBREMIDI_INLINE
std::vector<libremidi::output_port> observer_emscripten::get_output_ports() const noexcept
{
  std::vector<libremidi::output_port> ret;
  auto& webmidi = webmidi_helpers::midi_access_emscripten::instance();
  webmidi.load_current_infos();

  for (std::size_t i = 0, n = webmidi.output_count(); i < n; i++)
  {
    ret.push_back(to_port_info<false>(i, webmidi.outputs()[i]));
  }

  return ret;
}

LIBREMIDI_INLINE void observer_emscripten::update(
    const std::vector<observer_emscripten::device>& current_inputs,
    const std::vector<observer_emscripten::device>& current_outputs)
{
  // WebMIDI never remove inputs, it just marks them as disconnected.
  // At least in known browsers...
  assert(current_inputs.size() >= m_known_inputs.size());
  assert(current_outputs.size() >= m_known_outputs.size());

  for (std::size_t i = m_known_inputs.size(); i < current_inputs.size(); i++)
  {
    m_known_inputs.push_back(current_inputs[i]);
    configuration.input_added(to_port_info<true>(i, m_known_inputs[i]));
  }

  for (std::size_t i = m_known_outputs.size(); i < current_outputs.size(); i++)
  {
    m_known_outputs.push_back(current_outputs[i]);
    configuration.output_added(to_port_info<false>(i, m_known_outputs[i]));
  }
}
}
#endif
