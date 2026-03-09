#pragma once
#include <libremidi/api.hpp>
#include <libremidi/config.hpp>
#include <libremidi/error_handler.hpp>
#include <libremidi/observer_configuration.hpp>

#include <memory>
#include <vector>

NAMESPACE_LIBREMIDI
{
class observer_api
{
public:
  virtual ~observer_api() = default;

  virtual libremidi::API get_current_api() const noexcept = 0;
  virtual std::vector<libremidi::input_port> get_input_ports() const noexcept = 0;
  virtual std::vector<libremidi::output_port> get_output_ports() const noexcept = 0;
};

template <typename T, typename Arg>
std::unique_ptr<observer_api> make(libremidi::observer_configuration&& conf, Arg&& arg)
{
  return std::make_unique<T>(std::move(conf), std::move(arg));
}
}
