#if !defined(LIBREMIDI_HEADER_ONLY)
  #include <libremidi/libremidi.hpp>
#endif

#include <libremidi/backends.hpp>
#include <libremidi/detail/midi_api.hpp>

#include <array>
#include <cassert>

NAMESPACE_LIBREMIDI
{
LIBREMIDI_STATIC_INLINE_IMPLEMENTATION std::unique_ptr<midi_out_api>
make_midi_out_impl(auto base_conf, output_api_configuration api_conf)
{
  std::unique_ptr<midi_out_api> ptr;
  auto from_api = [&]<typename T>(T& /*backend*/) mutable {
    if (auto conf = get_if<typename T::midi_out_configuration>(&api_conf))
    {
      ptr = libremidi::make<typename T::midi_out>(std::move(base_conf), std::move(*conf));
      return true;
    }
    return false;
  };
  std::apply([&](auto&&... b) { (from_api(b) || ...); }, midi1::available_backends);
  if (!ptr)
    std::apply([&](auto&&... b) { (from_api(b) || ...); }, midi2::available_backends);
  return ptr;
}

LIBREMIDI_STATIC_INLINE_IMPLEMENTATION std::unique_ptr<midi_out_api>
make_midi_out(const output_configuration& base_conf)
{
  for (const auto& api : available_apis())
  {
    try
    {
      if (auto ret = make_midi_out_impl(base_conf, midi_out_configuration_for(api)))
        return ret;
    }
    catch (const std::exception& e)
    {
    }
  }

  for (const auto& api : available_ump_apis())
  {
    try
    {
      if (auto ret = make_midi_out_impl(base_conf, midi_out_configuration_for(api)))
        return ret;
    }
    catch (const std::exception& e)
    {
    }
  }

  return std::make_unique<midi_out_dummy>(output_configuration{}, dummy_configuration{});
}

LIBREMIDI_STATIC_INLINE_IMPLEMENTATION std::unique_ptr<midi_out_api>
make_midi_out(const output_configuration& base_conf, const output_api_configuration& api_conf)
{
  if (get_if<unspecified_configuration>(&api_conf))
  {
    return make_midi_out(base_conf);
  }
  else if (auto api_p = get_if<libremidi::API>(&api_conf))
  {
    if (*api_p == libremidi::API::UNSPECIFIED)
    {
      return make_midi_out(base_conf);
    }
    else
    {
      return make_midi_out_impl(base_conf, midi_out_configuration_for(*api_p));
    }
  }
  else
  {
    if (auto api = libremidi::midi_api(api_conf); api == libremidi::API::UNSPECIFIED)
      return make_midi_out(base_conf);
    else
      return make_midi_out_impl(base_conf, api_conf);
  }
}

LIBREMIDI_INLINE midi_out::midi_out(const output_configuration& base_conf) noexcept
    : m_impl{make_midi_out(base_conf)}
{
}

LIBREMIDI_INLINE
midi_out::midi_out(const output_configuration& base_conf, const output_api_configuration& api_conf)
    : m_impl{make_midi_out(base_conf, api_conf)}
{
  if (!m_impl)
  {
    error_handler e;
    e.libremidi_handle_error(base_conf, "Could not open midi out for the given api");
    m_impl = std::make_unique<midi_out_dummy>(output_configuration{}, dummy_configuration{});
  }
}

LIBREMIDI_INLINE midi_out::~midi_out() = default;

LIBREMIDI_INLINE midi_out::midi_out(midi_out&& other) noexcept
    : m_impl{std::move(other.m_impl)}
{
  other.m_impl
      = std::make_unique<libremidi::midi_out_dummy>(output_configuration{}, dummy_configuration{});
}

LIBREMIDI_INLINE midi_out& midi_out::operator=(midi_out&& other) noexcept
{
  this->m_impl = std::move(other.m_impl);
  other.m_impl
      = std::make_unique<libremidi::midi_out_dummy>(output_configuration{}, dummy_configuration{});
  return *this;
}

LIBREMIDI_INLINE
stdx::error midi_out::set_port_name(std::string_view portName) const
{
  if (m_impl->is_port_open())
    return m_impl->set_port_name(portName);

  return std::errc::not_connected;
}

LIBREMIDI_INLINE
libremidi::API midi_out::get_current_api() const noexcept
{
  return m_impl->get_current_api();
}

LIBREMIDI_INLINE
stdx::error midi_out::open_port(const output_port& port, std::string_view portName) const
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
stdx::error midi_out::open_virtual_port(std::string_view portName) const
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
stdx::error midi_out::close_port() const
{
  if (auto err = m_impl->is_client_open(); err != stdx::error{})
    return std::errc::not_connected;

  auto ret = m_impl->close_port();
  m_impl->connected_ = false;
  m_impl->port_open_ = false;
  return ret;
}

LIBREMIDI_INLINE
bool midi_out::is_port_open() const noexcept
{
  return m_impl->is_port_open();
}

LIBREMIDI_INLINE
bool midi_out::is_port_connected() const noexcept
{
  return m_impl->is_port_connected();
}

LIBREMIDI_INLINE
stdx::error midi_out::send_message(const libremidi::message& message) const
{
  return send_message(message.bytes.data(), message.bytes.size());
}

LIBREMIDI_INLINE
stdx::error midi_out::send_message(std::span<const unsigned char> message) const
{
  return send_message(message.data(), message.size());
}

LIBREMIDI_INLINE
stdx::error midi_out::send_message(unsigned char b0) const
{
  return send_message(&b0, 1);
}

LIBREMIDI_INLINE
stdx::error midi_out::send_message(unsigned char b0, unsigned char b1) const
{
  return send_message(std::to_array({b0, b1}));
}

LIBREMIDI_INLINE
stdx::error midi_out::send_message(unsigned char b0, unsigned char b1, unsigned char b2) const
{
  return send_message(std::to_array({b0, b1, b2}));
}

LIBREMIDI_INLINE
stdx::error midi_out::send_message(const unsigned char* message, size_t size) const
{
#if defined(LIBREMIDI_ASSERTIONS)
  assert(size > 0);
#endif

  return m_impl->send_message(message, size);
}

LIBREMIDI_INLINE
int64_t midi_out::current_time()
{
  return m_impl->current_time();
}

LIBREMIDI_INLINE
stdx::error midi_out::schedule_message(int64_t ts, const unsigned char* message, size_t size) const
{
#if defined(LIBREMIDI_ASSERTIONS)
  assert(size > 0);
#endif

  return m_impl->schedule_message(ts, message, size);
}

LIBREMIDI_INLINE
stdx::error midi_out::send_ump(const uint32_t* message, size_t size) const
{
#if defined(LIBREMIDI_ASSERTIONS)
  assert(size > 0);
  assert(size <= 4);
#endif

  return m_impl->send_ump(message, size);
}
LIBREMIDI_INLINE
stdx::error midi_out::send_ump(const libremidi::ump& message) const
{
  return send_ump(message.data, message.size());
}

LIBREMIDI_INLINE
stdx::error midi_out::send_ump(std::span<const uint32_t> message) const
{
  return send_ump(message.data(), message.size());
}

LIBREMIDI_INLINE
stdx::error midi_out::send_ump(uint32_t b0) const
{
  return send_ump(&b0, 1);
}

LIBREMIDI_INLINE
stdx::error midi_out::send_ump(uint32_t b0, uint32_t b1) const
{
  return send_ump(std::to_array({b0, b1}));
}

LIBREMIDI_INLINE
stdx::error midi_out::send_ump(uint32_t b0, uint32_t b1, uint32_t b2) const
{
  return send_ump(std::to_array({b0, b1, b2}));
}

LIBREMIDI_INLINE
stdx::error midi_out::send_ump(uint32_t b0, uint32_t b1, uint32_t b2, uint32_t b3) const
{
  return send_ump(std::to_array({b0, b1, b2, b3}));
}

LIBREMIDI_INLINE
stdx::error midi_out::send_ump(int32_t b0) const
{
  return send_ump(reinterpret_cast<uint32_t*>(&b0), 1);
}

LIBREMIDI_INLINE
stdx::error midi_out::send_ump(int64_t b01) const
{
  auto ptr = reinterpret_cast<uint32_t*>(&b01);
  uint32_t msg[2]{ptr[1], ptr[0]};
  return send_ump(msg, 2);
}

LIBREMIDI_INLINE
stdx::error midi_out::send_ump(uint64_t b01) const
{
  auto ptr = reinterpret_cast<uint32_t*>(&b01);
  uint32_t msg[2]{ptr[1], ptr[0]};
  return send_ump(msg, 2);
}

LIBREMIDI_INLINE
stdx::error midi_out::schedule_ump(int64_t ts, const uint32_t* message, size_t size) const
{
#if defined(LIBREMIDI_ASSERTIONS)
  assert(size > 0);
#endif

  return m_impl->schedule_ump(ts, message, size);
}
}
