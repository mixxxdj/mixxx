#if !defined(LIBREMIDI_HEADER_ONLY)
  #include <libremidi/libremidi.hpp>
#endif

#include <libremidi/backends.hpp>
#include <libremidi/detail/midi_api.hpp>

#include <cmath>

NAMESPACE_LIBREMIDI
{

LIBREMIDI_INLINE
std::string_view get_version() noexcept
{
  return LIBREMIDI_VERSION;
}

LIBREMIDI_INLINE std::string_view get_api_name(libremidi::API api)
{
  std::string_view ret;
  midi_any::for_backend(api, [&](auto& b) { ret = b.name; });
  return ret;
}

LIBREMIDI_INLINE std::string_view get_api_display_name(libremidi::API api)
{
  std::string_view ret;
  midi_any::for_backend(api, [&](auto& b) { ret = b.display_name; });
  return ret;
}

LIBREMIDI_INLINE libremidi::API get_compiled_api_by_name(std::string_view name)
{
  libremidi::API ret = libremidi::API::UNSPECIFIED;
  midi_any::for_all_backends([&](auto& b) {
    if (name == b.name)
      ret = b.API;
  });
  return ret;
}

[[nodiscard]] LIBREMIDI_INLINE std::vector<libremidi::API> available_apis() noexcept
{
  std::vector<libremidi::API> apis;
  midi1::for_all_backends([&](auto b) { apis.push_back(b.API); });
  return apis;
}

[[nodiscard]] LIBREMIDI_INLINE std::vector<libremidi::API> available_ump_apis() noexcept
{
  std::vector<libremidi::API> apis;
  midi2::for_all_backends([&](auto b) { apis.push_back(b.API); });
  return apis;
}

LIBREMIDI_INLINE
libremidi::API midi_api(const input_api_configuration& conf)
{
  libremidi::API ret = libremidi::API::UNSPECIFIED;
  midi_any::for_all_backends([&]<typename T>(T) {
    if (get_if<typename T::midi_in_configuration>(&conf))
    {
      ret = T::API;
    }
  });
  return ret;
}
LIBREMIDI_INLINE
libremidi::API midi_api(const output_api_configuration& conf)
{
  libremidi::API ret = libremidi::API::UNSPECIFIED;
  midi_any::for_all_backends([&]<typename T>(T) {
    if (get_if<typename T::midi_out_configuration>(&conf))
    {
      ret = T::API;
    }
  });
  return ret;
}
LIBREMIDI_INLINE
libremidi::API midi_api(const observer_api_configuration& conf)
{
  libremidi::API ret = libremidi::API::UNSPECIFIED;
  midi_any::for_all_backends([&]<typename T>(T) {
    if (get_if<typename T::midi_observer_configuration>(&conf))
    {
      ret = T::API;
    }
  });
  return ret;
}

LIBREMIDI_INLINE
input_api_configuration midi_in_configuration_for(libremidi::API api)
{
  input_api_configuration ret;
  midi_any::for_backend(api, [&]<typename T>(T) {
    using conf_type = typename T::midi_in_configuration;
    ret = conf_type{};
  });
  return ret;
}

LIBREMIDI_INLINE
output_api_configuration midi_out_configuration_for(libremidi::API api)
{
  output_api_configuration ret;
  midi_any::for_backend(api, [&]<typename T>(T) {
    using conf_type = typename T::midi_out_configuration;
    ret = conf_type{};
  });
  return ret;
}

LIBREMIDI_INLINE
observer_api_configuration observer_configuration_for(libremidi::API api)
{
  observer_api_configuration ret;
  midi_any::for_backend(api, [&]<typename T>(T) {
    using conf_type = typename T::midi_observer_configuration;
    ret = conf_type{};
  });
  return ret;
}

LIBREMIDI_INLINE
input_api_configuration midi_in_configuration_for(const libremidi::observer& obs)
{
  return midi_in_configuration_for(obs.get_current_api());
}

LIBREMIDI_INLINE
output_api_configuration midi_out_configuration_for(const libremidi::observer& obs)
{
  // FIXME reuse context when meaningful, e.g. ALSA, JACK...
  return midi_out_configuration_for(obs.get_current_api());
}

LIBREMIDI_INLINE
std::optional<input_port> in_default_port(libremidi::API api) noexcept
try
{
  libremidi::observer obs{{}, observer_configuration_for(api)};
  if (auto ports = obs.get_input_ports(); !ports.empty())
    return ports.front();
  return std::nullopt;
}
catch (const std::exception& e)
{
  return std::nullopt;
}

LIBREMIDI_INLINE
std::optional<output_port> out_default_port(libremidi::API api) noexcept
try
{
  libremidi::observer obs{{}, observer_configuration_for(api)};
  if (auto ports = obs.get_output_ports(); !ports.empty())
    return ports.front();
  return std::nullopt;
}
catch (const std::exception& e)
{
  return std::nullopt;
}

namespace midi1
{
LIBREMIDI_INLINE
input_api_configuration in_default_configuration()
{
  return midi_in_configuration_for(default_api());
}

LIBREMIDI_INLINE
output_api_configuration out_default_configuration()
{
  return midi_out_configuration_for(default_api());
}

LIBREMIDI_INLINE
observer_api_configuration observer_default_configuration()
{
  return observer_configuration_for(default_api());
}
}

namespace midi2
{
LIBREMIDI_INLINE
input_api_configuration in_default_configuration()
{
  return midi_in_configuration_for(default_api());
}

LIBREMIDI_INLINE
output_api_configuration out_default_configuration()
{
  return midi_out_configuration_for(default_api());
}

LIBREMIDI_INLINE
observer_api_configuration observer_default_configuration()
{
  return observer_configuration_for(default_api());
}
}

}
