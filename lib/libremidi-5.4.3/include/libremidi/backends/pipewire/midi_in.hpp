#pragma once
#include <libremidi/backends/pipewire/config.hpp>
#include <libremidi/backends/pipewire/helpers.hpp>
#include <libremidi/detail/midi_in.hpp>
#include <libremidi/detail/midi_stream_decoder.hpp>

NAMESPACE_LIBREMIDI
{
class midi_in_pipewire final
    : public midi1::in_api
    , public pipewire_helpers
    , public error_handler
{
public:
  struct
      : input_configuration
      , pipewire_input_configuration
  {
  } configuration;

  explicit midi_in_pipewire(input_configuration&& conf, pipewire_input_configuration&& apiconf)
      : configuration{std::move(conf), std::move(apiconf)}
  {
    if (auto ret = create_context(*this); ret != stdx::error{})
    {
      client_open_ = ret;
      return;
    }
    if (auto ret = create_filter(*this); ret != stdx::error{})
    {
      client_open_ = ret;
      return;
    }
    client_open_ = stdx::error{};
  }

  ~midi_in_pipewire() override
  {
    stop_thread();
    do_close_port();
    destroy_filter(*this);
    destroy_context();
    client_open_ = std::errc::not_connected;
  }

  libremidi::API get_current_api() const noexcept override { return libremidi::API::PIPEWIRE; }

  stdx::error open_port(const input_port& in_port, std::string_view name) override
  {
    if (auto err = create_local_port(*this, name, SPA_DIRECTION_INPUT, "8 bit raw midi");
        err != stdx::error{})
      return err;

    if (auto err = link_ports(*this, in_port); err != stdx::error{})
      return err;

    start_thread();
    return stdx::error{};
  }

  stdx::error open_virtual_port(std::string_view name) override
  {
    if (auto err = create_local_port(*this, name, SPA_DIRECTION_INPUT, "8 bit raw midi");
        err != stdx::error{})
      return err;

    start_thread();
    return stdx::error{};
  }

  stdx::error close_port() override
  {
    stop_thread();
    return do_close_port();
  }

  stdx::error set_port_name(std::string_view port_name) override { return rename_port(port_name); }

  timestamp absolute_timestamp() const noexcept override { return system_ns(); }

  void process(struct spa_io_position* position)
  {
    static constexpr timestamp_backend_info timestamp_info{
        .has_absolute_timestamps = true,
        .absolute_is_monotonic = true,
        .has_samples = true,
    };

    assert(this->filter);
    assert(this->filter->port);
    const auto b = pw.filter_dequeue_buffer(this->filter->port);
    if (!b)
      return;

    const auto buf = b->buffer;
    const auto d = &buf->datas[0];

    if (d->data == nullptr)
      return;

    const auto pod
        = (spa_pod*)spa_pod_from_data(d->data, d->maxsize, d->chunk->offset, d->chunk->size);
    if (!pod)
      return;
    if (!spa_pod_is_sequence(pod))
      return;

    struct spa_pod_control* c{};
    SPA_POD_SEQUENCE_FOREACH((struct spa_pod_sequence*)pod, c)
    {
      if (c->type != SPA_CONTROL_Midi)
        continue;

      auto data = (uint8_t*)SPA_POD_BODY(&c->value);
      auto size = SPA_POD_BODY_SIZE(&c->value);

      const auto to_ns = [=, clk = position->clock] {
        return 1e9 * ((clk.position + c->offset) / (double)clk.rate.denom);
      };

      m_processing.on_bytes(
          {data, data + size}, m_processing.timestamp<timestamp_info>(to_ns, c->offset));
    }

    pw.filter_queue_buffer(this->filter->port, b);
  }

  midi1::input_state_machine m_processing{this->configuration};
};
}
