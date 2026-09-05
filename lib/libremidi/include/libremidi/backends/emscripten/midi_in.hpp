#pragma once
#include <libremidi/backends/emscripten/config.hpp>
#include <libremidi/backends/emscripten/helpers.hpp>
#include <libremidi/detail/midi_in.hpp>
#include <libremidi/detail/midi_stream_decoder.hpp>

NAMESPACE_LIBREMIDI
{
class midi_in_emscripten final
    : public midi1::in_api
    , public error_handler
{
public:
  struct
      : input_configuration
      , emscripten_input_configuration
  {
  } configuration;

  midi_in_emscripten(input_configuration&& conf, emscripten_input_configuration&& apiconf);
  ~midi_in_emscripten() override;

  libremidi::API get_current_api() const noexcept override;

  stdx::error open_port(int portNumber, std::string_view);
  stdx::error open_port(const input_port& p, std::string_view) override;
  stdx::error close_port() override;

  timestamp absolute_timestamp() const noexcept override;

  void on_input(double ts, unsigned char* begin, unsigned char* end);

private:
  int m_portNumber{};

  midi1::input_state_machine m_processing{this->configuration};
};
}
