#pragma once
#include <libremidi/detail/midi_api.hpp>
#include <libremidi/error_handler.hpp>
#include <libremidi/input_configuration.hpp>
#include <libremidi/observer_configuration.hpp>

NAMESPACE_LIBREMIDI
{
class midi_in_api : public midi_api
{
public:
  midi_in_api() = default;
  ~midi_in_api() override = default;
  midi_in_api(const midi_in_api&) = delete;
  midi_in_api(midi_in_api&&) = delete;
  midi_in_api& operator=(const midi_in_api&) = delete;
  midi_in_api& operator=(midi_in_api&&) = delete;

  [[nodiscard]] virtual stdx::error
  open_port(const input_port& pt, std::string_view local_port_name)
      = 0;
  [[nodiscard]] virtual timestamp absolute_timestamp() const noexcept = 0;
};

namespace midi1
{
class in_api : public midi_in_api
{
public:
  using midi_in_api::midi_in_api;
};
}

namespace midi2
{
class in_api : public midi_in_api
{
public:
  using midi_in_api::midi_in_api;

  bool firstMessage{true};
};
}

template <typename T, typename Arg>
std::unique_ptr<midi_in_api> make(libremidi::input_configuration&& conf, Arg&& arg)
{
  return std::make_unique<T>(std::move(conf), std::move(arg));
}

template <typename T, typename Arg>
std::unique_ptr<midi_in_api> make(libremidi::ump_input_configuration&& conf, Arg&& arg)
{
  return std::make_unique<T>(std::move(conf), std::move(arg));
}
}
