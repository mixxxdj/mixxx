#pragma once
#include <libremidi/backends/rawio/config.hpp>
#include <libremidi/detail/midi_out.hpp>

NAMESPACE_LIBREMIDI::rawio_ump
{
class midi_out final
    : public midi2::out_api
    , public error_handler
{
public:
  struct
      : output_configuration
      , rawio_ump_output_configuration
  {
  } configuration;

  explicit midi_out(output_configuration&& conf, rawio_ump_output_configuration&& apiconf)
      : configuration{std::move(conf), std::move(apiconf)}
  {
    client_open_ = stdx::error{};
  }

  ~midi_out() override { close_port(); }

  libremidi::API get_current_api() const noexcept override { return libremidi::API::RAW_IO_UMP; }

  stdx::error open_port(const output_port&, std::string_view) override { return stdx::error{}; }

  stdx::error open_virtual_port(std::string_view) override { return stdx::error{}; }

  stdx::error close_port() override
  {
    configuration.write_ump = nullptr;
    return stdx::error{};
  }

  stdx::error set_port_name(std::string_view) override { return stdx::error{}; }

  stdx::error send_ump(const uint32_t* message, size_t size) override
  {
    return configuration.write_ump({message, size});
  }
};
}
