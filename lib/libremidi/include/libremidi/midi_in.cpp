#if !defined(LIBREMIDI_HEADER_ONLY)
  #include <libremidi/libremidi.hpp>
#endif

#include <libremidi/backends.hpp>
#include <libremidi/detail/midi_api.hpp>

#include <cassert>

NAMESPACE_LIBREMIDI
{

LIBREMIDI_STATIC_IMPLEMENTATION libremidi::ump_input_configuration
convert_midi1_to_midi2_input_configuration(const input_configuration& base_conf) noexcept
{
  libremidi::ump_input_configuration c2;
  c2.on_message = [cb = base_conf.on_message,
                   converter = midi2_to_midi1{}](libremidi::ump&& msg) mutable -> void {
    converter.convert(
        msg.data, 1, msg.timestamp, [cb](const unsigned char* midi, std::size_t n, int64_t ts) {
      cb(libremidi::message{{midi, midi + n}, ts});
      return stdx::error{};
    });
  };
  c2.get_timestamp = base_conf.get_timestamp;
  c2.on_error = base_conf.on_error;
  c2.on_warning = base_conf.on_warning;
  c2.ignore_sysex = base_conf.ignore_sysex;
  c2.ignore_timing = base_conf.ignore_timing;
  c2.ignore_sensing = base_conf.ignore_sensing;
  c2.timestamps = base_conf.timestamps;
  return c2;
}

LIBREMIDI_STATIC_IMPLEMENTATION libremidi::input_configuration
convert_midi2_to_midi1_input_configuration(const ump_input_configuration& base_conf) noexcept
{
  libremidi::input_configuration c2;
  c2.on_message = [cb = base_conf.on_message,
                   converter = midi1_to_midi2{}](libremidi::message&& msg) mutable -> void {
    converter.convert(
        msg.bytes.data(), msg.bytes.size(), msg.timestamp,
        [cb](const uint32_t* ump, std::size_t n, int64_t ts) {
      libremidi::ump u{ump[0]};
      for (std::size_t i = 1; i < n && i < 4; i++)
        u.data[i] = ump[i];
      u.timestamp = ts;
      cb(std::move(u));
      return stdx::error{};
    });
  };
  c2.get_timestamp = base_conf.get_timestamp;
  c2.on_error = base_conf.on_error;
  c2.on_warning = base_conf.on_warning;
  c2.ignore_sysex = base_conf.ignore_sysex;
  c2.ignore_timing = base_conf.ignore_timing;
  c2.ignore_sensing = base_conf.ignore_sensing;
  c2.timestamps = base_conf.timestamps;
  return c2;
}

LIBREMIDI_STATIC_INLINE_IMPLEMENTATION std::unique_ptr<midi_in_api>
make_midi_in(auto base_conf, input_api_configuration api_conf, auto backends)
{
  std::unique_ptr<midi_in_api> ptr;

  assert(base_conf.on_message || base_conf.on_raw_data);

  auto from_api = [&]<typename T>(T& /*backend*/) mutable {
    if (auto conf = get_if<typename T::midi_in_configuration>(&api_conf))
    {
      ptr = libremidi::make<typename T::midi_in>(std::move(base_conf), std::move(*conf));
      return true;
    }
    return false;
  };
  std::apply([&](auto&&... b) { (from_api(b) || ...); }, backends);
  return ptr;
}

/// MIDI 1 helpers
LIBREMIDI_STATIC_INLINE_IMPLEMENTATION std::unique_ptr<midi_in_api>
make_midi1_in(const input_configuration& base_conf)
{
  for (const auto& api : available_apis())
  {
    try
    {
      auto impl
          = make_midi_in(base_conf, midi_in_configuration_for(api), midi1::available_backends);
      if (impl)
        return impl;
    }
    catch (const std::exception& e)
    {
    }
  }

  // No MIDI 1 backend, try the MIDI 2 ones with a wrap:
  {
    auto c2 = convert_midi1_to_midi2_input_configuration(base_conf);
    for (const auto& api : available_ump_apis())
    {
      try
      {
        auto impl = make_midi_in(c2, midi_in_configuration_for(api), midi2::available_backends);
        if (impl)
          return impl;
      }
      catch (const std::exception& e)
      {
      }
    }
  }

  return std::make_unique<midi_in_dummy>(input_configuration{}, dummy_configuration{});
}

LIBREMIDI_STATIC_INLINE_IMPLEMENTATION std::unique_ptr<midi_in_api> make_midi1_in(
    const input_configuration& base_conf, const input_api_configuration& api_conf,
    libremidi::API api)
{
  if (libremidi::is_midi1(api))
  {
    return make_midi_in(base_conf, api_conf, midi1::available_backends);
  }
  else if (libremidi::is_midi2(api))
  {
    auto c2 = convert_midi1_to_midi2_input_configuration(base_conf);
    return make_midi_in(c2, api_conf, midi2::available_backends);
  }
  return {};
}

LIBREMIDI_STATIC_INLINE_IMPLEMENTATION std::unique_ptr<midi_in_api>
make_midi1_in(const input_configuration& base_conf, const input_api_configuration& api_conf)
{
  if (get_if<unspecified_configuration>(&api_conf))
  {
    return make_midi1_in(base_conf);
  }
  else if (auto api_p = get_if<libremidi::API>(&api_conf))
  {
    if (*api_p == libremidi::API::UNSPECIFIED)
    {
      return make_midi1_in(base_conf);
    }
    else
    {
      return make_midi1_in(base_conf, midi_in_configuration_for(*api_p), *api_p);
    }
  }
  else
  {
    if (auto api = libremidi::midi_api(api_conf); api == libremidi::API::UNSPECIFIED)
      return {};
    else
      return make_midi1_in(base_conf, api_conf, libremidi::midi_api(api_conf));
  }
}

/// MIDI 1 constructors
LIBREMIDI_INLINE midi_in::midi_in(const input_configuration& base_conf) noexcept
    : m_impl{make_midi1_in(base_conf)}
{
}

LIBREMIDI_INLINE
midi_in::midi_in(const input_configuration& base_conf, const input_api_configuration& api_conf)
    : m_impl{make_midi1_in(base_conf, api_conf)}
{
  if (!m_impl)
  {
    error_handler e;
    e.libremidi_handle_error(base_conf, "Could not open midi in for the given api");
    m_impl = std::make_unique<midi_in_dummy>(input_configuration{}, dummy_configuration{});
  }
}

/// MIDI 2 helpers
LIBREMIDI_STATIC_INLINE_IMPLEMENTATION std::unique_ptr<midi_in_api>
make_midi2_in(const ump_input_configuration& base_conf)
{
  for (const auto& api : available_ump_apis())
  {
    try
    {
      if (auto ret
          = make_midi_in(base_conf, midi_in_configuration_for(api), midi2::available_backends))
        return ret;
    }
    catch (const std::exception& e)
    {
    }
  }

  // No MIDI 2 backend, try the MIDI 1 ones with a wrap:
  {
    auto c2 = convert_midi2_to_midi1_input_configuration(base_conf);
    for (const auto& api : available_apis())
    {
      try
      {
        if (auto ret = make_midi_in(c2, midi_in_configuration_for(api), midi1::available_backends))
          return ret;
      }
      catch (const std::exception& e)
      {
      }
    }
  }

  return {};
}

LIBREMIDI_STATIC_INLINE_IMPLEMENTATION std::unique_ptr<midi_in_api> make_midi2_in(
    const ump_input_configuration& base_conf, const input_api_configuration& api_conf,
    libremidi::API api)
{
  if (is_midi2(api))
  {
    return make_midi_in(base_conf, api_conf, midi2::available_backends);
  }
  else if (is_midi1(api))
  {
    auto c2 = convert_midi2_to_midi1_input_configuration(base_conf);
    return make_midi_in(c2, api_conf, midi1::available_backends);
  }

  return {};
}

LIBREMIDI_STATIC_INLINE_IMPLEMENTATION std::unique_ptr<midi_in_api>
make_midi2_in(const ump_input_configuration& base_conf, const input_api_configuration& api_conf)
{
  if (get_if<unspecified_configuration>(&api_conf))
  {
    return make_midi2_in(base_conf);
  }
  else if (auto api_p = get_if<libremidi::API>(&api_conf))
  {
    if (*api_p == libremidi::API::UNSPECIFIED)
    {
      return make_midi2_in(base_conf);
    }
    else
    {
      return make_midi2_in(base_conf, midi_in_configuration_for(*api_p), *api_p);
    }
  }
  else
  {
    if (auto api = libremidi::midi_api(api_conf); api == libremidi::API::UNSPECIFIED)
      return {};
    else
      return make_midi2_in(base_conf, api_conf, libremidi::midi_api(api_conf));
  }
}

/// MIDI 2 constructors
LIBREMIDI_INLINE midi_in::midi_in(const ump_input_configuration& base_conf) noexcept
    : m_impl{make_midi2_in(base_conf)}
{
}

LIBREMIDI_INLINE
midi_in::midi_in(const ump_input_configuration& base_conf, const input_api_configuration& api_conf)
    : m_impl{make_midi2_in(base_conf, api_conf)}
{
  if (!m_impl)
  {
    error_handler e;
    e.libremidi_handle_error(base_conf, "Could not open midi in for the given api");
    m_impl = std::make_unique<midi_in_dummy>(input_configuration{}, dummy_configuration{});
  }
}

LIBREMIDI_INLINE midi_in::~midi_in() = default;

LIBREMIDI_INLINE midi_in::midi_in(midi_in&& other) noexcept
    : m_impl{std::move(other.m_impl)}
{
  other.m_impl
      = std::make_unique<libremidi::midi_in_dummy>(input_configuration{}, dummy_configuration{});
}

LIBREMIDI_INLINE
stdx::error midi_in::set_port_name(std::string_view portName)
{
  if (m_impl->is_port_open())
    return m_impl->set_port_name(portName);

  return std::errc::not_connected;
}

LIBREMIDI_INLINE midi_in& midi_in::operator=(midi_in&& other) noexcept
{
  this->m_impl = std::move(other.m_impl);
  other.m_impl
      = std::make_unique<libremidi::midi_in_dummy>(input_configuration{}, dummy_configuration{});
  return *this;
}

LIBREMIDI_INLINE
libremidi::API midi_in::get_current_api() const noexcept
{
  return m_impl->get_current_api();
}

LIBREMIDI_INLINE
stdx::error midi_in::open_port(const input_port& port, std::string_view portName)
{
  if (port.api != get_current_api())
    return std::errc::invalid_argument;

  if (auto err = m_impl->is_client_open(); err != stdx::error{})
    return std::errc::not_connected;

  if (m_impl->is_port_open())
    return std::errc::operation_not_supported;

  auto ret = m_impl->open_port(port, portName);
  if (ret == stdx::error{})
  {
    m_impl->connected_ = true;
    m_impl->port_open_ = true;
  }
  return ret;
}

LIBREMIDI_INLINE
stdx::error midi_in::open_virtual_port(std::string_view portName)
{
  if (auto err = m_impl->is_client_open(); err != stdx::error{})
    return std::errc::not_connected;

  if (m_impl->is_port_open())
    return std::errc::operation_not_supported;

  auto ret = m_impl->open_virtual_port(portName);
  if (ret == stdx::error{})
    m_impl->port_open_ = true;
  return ret;
}

LIBREMIDI_INLINE
stdx::error midi_in::close_port()
{
  if (auto err = m_impl->is_client_open(); err != stdx::error{})
    return std::errc::not_connected;

  auto ret = m_impl->close_port();

  m_impl->connected_ = false;
  m_impl->port_open_ = false;

  return ret;
}

LIBREMIDI_INLINE
bool midi_in::is_port_open() const noexcept
{
  return m_impl->is_port_open();
}

LIBREMIDI_INLINE
bool midi_in::is_port_connected() const noexcept
{
  return m_impl->is_port_connected();
}

LIBREMIDI_INLINE
int64_t midi_in::absolute_timestamp() const noexcept
{
  return m_impl->absolute_timestamp();
}
}
