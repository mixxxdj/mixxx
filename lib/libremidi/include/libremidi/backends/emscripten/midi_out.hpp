#pragma once
#include <libremidi/backends/emscripten/config.hpp>
#include <libremidi/backends/emscripten/helpers.hpp>
#include <libremidi/detail/midi_out.hpp>

NAMESPACE_LIBREMIDI
{
class midi_out_emscripten final
    : public midi1::out_api
    , public error_handler
{
public:
  struct
      : output_configuration
      , emscripten_output_configuration
  {
  } configuration;

  midi_out_emscripten(output_configuration&& conf, emscripten_output_configuration&& apiconf);
  ~midi_out_emscripten() override;

  libremidi::API get_current_api() const noexcept override;

  stdx::error open_port(int portNumber, std::string_view);
  stdx::error open_port(const output_port& p, std::string_view) override;
  stdx::error close_port() override;

  stdx::error send_message(const unsigned char* message, size_t size) override;

private:
  int m_portNumber{-1};
};
}
