#pragma once
#include <libremidi/backends/emscripten/config.hpp>
#include <libremidi/backends/emscripten/helpers.hpp>
#include <libremidi/detail/observer.hpp>

#include <vector>

NAMESPACE_LIBREMIDI
{
class observer_emscripten final : public observer_api
{
  using device = webmidi_helpers::device_information;

public:
  struct
      : observer_configuration
      , emscripten_observer_configuration
  {
  } configuration;

  explicit observer_emscripten(
      observer_configuration&& conf, emscripten_observer_configuration&& apiconf);
  ~observer_emscripten();

  void
  update(const std::vector<device>& current_inputs, const std::vector<device>& current_outputs);

  libremidi::API get_current_api() const noexcept override;

  std::vector<libremidi::input_port> get_input_ports() const noexcept override;
  std::vector<libremidi::output_port> get_output_ports() const noexcept override;

private:
  std::vector<device> m_known_inputs;
  std::vector<device> m_known_outputs;
};
}
