#pragma once
#include <libremidi/api.hpp>
#include <libremidi/config.hpp>
#include <libremidi/error.hpp>

#include <string_view>

NAMESPACE_LIBREMIDI
{
class midi_api
{
public:
  midi_api() = default;
  virtual ~midi_api() = default;
  midi_api(const midi_api&) = delete;
  midi_api(midi_api&&) = delete;
  midi_api& operator=(const midi_api&) = delete;
  midi_api& operator=(midi_api&&) = delete;

  [[nodiscard]] virtual libremidi::API get_current_api() const noexcept = 0;

  [[nodiscard]] virtual stdx::error open_virtual_port(std::string_view)
  {
    return std::errc::function_not_supported;
  }
  virtual stdx::error set_port_name(std::string_view) { return std::errc::function_not_supported; }

  virtual stdx::error close_port() = 0;

  stdx::error is_client_open() const noexcept { return client_open_; }
  bool is_port_open() const noexcept { return port_open_; }
  bool is_port_connected() const noexcept { return connected_; }

protected:
  friend class midi_in;
  friend class midi_out;
  stdx::error client_open_{std::errc::not_connected};
  bool port_open_{};
  bool connected_{};
};
}
