#pragma once
#include <libremidi/backends/pipewire/config.hpp>
#include <libremidi/backends/pipewire/helpers.hpp>
#include <libremidi/detail/midi_out.hpp>

#include <readerwriterqueue.h>

NAMESPACE_LIBREMIDI
{
class midi_out_pipewire
    : public midi1::out_api
    , public pipewire_helpers
    , public error_handler
{
public:
  struct
      : output_configuration
      , pipewire_output_configuration
  {
  } configuration;

  midi_out_pipewire(output_configuration&& conf, pipewire_output_configuration&& apiconf)
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

  ~midi_out_pipewire() override
  {
    stop_thread();
    do_close_port();
    destroy_filter(*this);
    destroy_context();
    client_open_ = std::errc::not_connected;
  }

  libremidi::API get_current_api() const noexcept override { return libremidi::API::PIPEWIRE; }

  stdx::error open_port(const output_port& out_port, std::string_view name) override
  {
    if (auto err = create_local_port(*this, name, SPA_DIRECTION_OUTPUT, "8 bit raw midi");
        err != stdx::error{})
      return err;

    this->filter->set_port_buffer(configuration.output_buffer_size);

    if (auto err = link_ports(*this, out_port); err != stdx::error{})
      return err;

    start_thread();
    return stdx::error{};
  }

  stdx::error open_virtual_port(std::string_view name) override
  {
    if (auto err = create_local_port(*this, name, SPA_DIRECTION_OUTPUT, "8 bit raw midi");
        err != stdx::error{})
      return err;

    this->filter->set_port_buffer(configuration.output_buffer_size);

    start_thread();
    return stdx::error{};
  }

  stdx::error close_port() override
  {
    stop_thread();
    return do_close_port();
  }

  stdx::error set_port_name(std::string_view port_name) override { return rename_port(port_name); }

  int process(spa_io_position* pos)
  {
    m_process_clock.store(pos->clock.nsec, std::memory_order_relaxed);
    const auto b = pw.filter_dequeue_buffer(this->filter->port);
    if (!b)
      return 1;

    const auto buf = b->buffer;
    const auto d = &buf->datas[0];

    if (d->data == nullptr)
      return 1;

    spa_pod_builder build;
    spa_zero(build);
    spa_pod_builder_init(&build, d->data, d->maxsize);

    spa_pod_frame f;
    spa_pod_builder_push_sequence(&build, &f, 0);

    // for all events
    while (auto m_ptr = m_queue.peek())
    {
      auto& m = *m_ptr;
      if (m.empty())
      {
        m_queue.pop();
        continue;
      }

      // TODO why
      if (m.bytes[0] == 0xff)
      {
        m_queue.pop();
        continue;
      }

      spa_pod_builder_control(&build, static_cast<int32_t>(m.timestamp), SPA_CONTROL_Midi);
      int res
          = spa_pod_builder_bytes(&build, m.bytes.data(), static_cast<uint32_t>(m.bytes.size()));

      // Try again next buffer
      if (res == -ENOSPC)
        break;

      // Recycle the memory
      m_gcqueue.enqueue(std::move(m));
      m_queue.pop();
    }
    spa_pod_builder_pop(&build, &f);

    int n_fill_frames = build.state.offset;
    if (n_fill_frames > 0)
    {
      d->chunk->offset = 0;
      d->chunk->stride = 1;
      d->chunk->size = n_fill_frames;
      b->size = n_fill_frames;

      pw.filter_queue_buffer(this->filter->port, b);
      return 0;
    }

    pw.filter_flush(this->filter->filter, true);

    return 0;
  }

  stdx::error send_message(const unsigned char* message, size_t size) override
  {
    libremidi::message m;
    m_gcqueue.try_dequeue(m);
    m.bytes.assign(message, message + size);
    m.timestamp = 0;
    m_queue.enqueue(std::move(m));
    return stdx::error{};
  }

  int convert_timestamp(int64_t user) const noexcept
  {
    switch (configuration.timestamps)
    {
      case timestamp_mode::AudioFrame:
        return static_cast<int>(user);

      default:
        // TODO
        return 0;
    }
  }

  stdx::error schedule_message(int64_t ts, const unsigned char* message, size_t size) override
  {
    m_queue.enqueue(
        libremidi::message(midi_bytes{message, message + size}, convert_timestamp(ts)));
    return stdx::error{};
  }

  moodycamel::ReaderWriterQueue<libremidi::message> m_queue;
  moodycamel::ReaderWriterQueue<libremidi::message> m_gcqueue;
  std::atomic_int64_t m_process_clock = 0;
};
}
