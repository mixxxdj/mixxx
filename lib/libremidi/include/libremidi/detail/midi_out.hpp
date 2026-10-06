#pragma once
#include <libremidi/detail/conversion.hpp>
#include <libremidi/detail/midi_api.hpp>
#include <libremidi/error_handler.hpp>
#include <libremidi/output_configuration.hpp>

#include <string_view>

NAMESPACE_LIBREMIDI
{

class midi_out_api : public midi_api
{
public:
  midi_out_api() = default;
  ~midi_out_api() override = default;
  midi_out_api(const midi_out_api&) = delete;
  midi_out_api(midi_out_api&&) = delete;
  midi_out_api& operator=(const midi_out_api&) = delete;
  midi_out_api& operator=(midi_out_api&&) = delete;

  [[nodiscard]] virtual stdx::error
  open_port(const output_port& pt, std::string_view local_port_name)
      = 0;

  [[nodiscard]] virtual int64_t current_time() const noexcept { return 0; }

  virtual stdx::error send_message(const unsigned char* message, std::size_t size) = 0;
  virtual stdx::error
  schedule_message(int64_t /*ts*/, const unsigned char* message, std::size_t size)
  {
    return send_message(message, size);
  }

  virtual stdx::error send_ump(const uint32_t* message, std::size_t size) = 0;
  virtual stdx::error schedule_ump(int64_t /*ts*/, const uint32_t* ump, std::size_t size)
  {
    return send_ump(ump, size);
  }
};

namespace midi1
{
class out_api : public midi_out_api
{
  friend struct midi_stream_decoder;

public:
  using midi_out_api::midi_out_api;

  stdx::error send_ump(const uint32_t* message, std::size_t size)
  {
    return converter.convert(
        message, size, 0, [this](const unsigned char* midi, std::size_t n, int64_t /* ts */) {
      return send_message(midi, n);
    });
  }

  midi2_to_midi1 converter;
};
}

namespace midi2
{
class out_api : public midi_out_api
{
  friend struct midi_stream_decoder;

public:
  using midi_out_api::midi_out_api;

  stdx::error send_message(const unsigned char* message, std::size_t size)
  {
    return converter.convert(
        message, size, 0, [this](const uint32_t* ump, std::size_t count, int64_t /* ts */) {
      return send_ump(ump, count);
    });
  }

  midi1_to_midi2 converter;
};
}

template <typename T, typename Arg>
std::unique_ptr<midi_out_api> make(libremidi::output_configuration&& conf, Arg&& arg)
{
  return std::make_unique<T>(std::move(conf), std::forward<Arg>(arg));
}
}
