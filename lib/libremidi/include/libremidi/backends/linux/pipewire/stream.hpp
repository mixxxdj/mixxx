#pragma once
#include <libremidi/backends/linux/pipewire/context.hpp>
#include <libremidi/backends/linux/pipewire/loader.hpp>
#include <libremidi/config.hpp>

#include <pipewire/keys.h>
#include <pipewire/properties.h>
#include <pipewire/stream.h>

#include <cstdint>
#include <memory>
#include <span>
#include <string>
#include <unordered_map>

namespace libremidi::pipewire
{

struct stream_config
{
  std::string name;

  pw_direction direction{PW_DIRECTION_INPUT};
  std::uint32_t target_id{PW_ID_ANY};
  pw_stream_flags flags{PW_STREAM_FLAG_AUTOCONNECT};
  std::unordered_map<std::string, std::string> properties;
};

class stream
{
public:
  using config = stream_config;

  stream(std::shared_ptr<context> ctx, config cfg, const pw_stream_events& events, void* user_data)
      : m_ctx{std::move(ctx)}
      , m_cfg{std::move(cfg)}
  {
    if (!m_ctx || !m_ctx->ok())
      return;
    auto& pw = load();
    if (!pw.stream_available)
      return;

    m_ctx->with_lock([&] {
      auto* props = build_stream_props(pw);
      if (!props)
        return;
      // props ownership taken by pw_stream_new_simple.
      m_stream = pw.stream_new_simple(
          m_ctx->bare_loop(), m_cfg.name.c_str(), props, &events, user_data);
    });
  }

  ~stream() { destroy(); }

  stream(const stream&) = delete;
  stream& operator=(const stream&) = delete;
  stream(stream&&) = delete;
  stream& operator=(stream&&) = delete;

  bool ok() const noexcept { return m_stream != nullptr; }

  int connect(std::span<const spa_pod*> params)
  {
    if (!ok())
      return -ENOTCONN;
    auto& pw = load();
    int rc = 0;
    m_ctx->with_lock([&] {
      rc = pw.stream_connect(
          m_stream, m_cfg.direction, m_cfg.target_id, m_cfg.flags, params.data(),
          static_cast<std::uint32_t>(params.size()));
    });
    return rc;
  }

  int update_params(std::span<const spa_pod*> params)
  {
    if (!ok())
      return -ENOTCONN;
    auto& pw = load();
    int rc = 0;
    m_ctx->with_lock([&] {
      rc = pw.stream_update_params(
          m_stream, params.data(), static_cast<std::uint32_t>(params.size()));
    });
    return rc;
  }

  int set_active(bool active)
  {
    if (!ok())
      return -ENOTCONN;
    auto& pw = load();
    int rc = 0;
    m_ctx->with_lock([&] {
      if (pw.stream_set_active)
        rc = pw.stream_set_active(m_stream, active);
    });
    return rc;
  }

  pw_stream_state state(const char** error_out = nullptr) const noexcept
  {
    if (!m_stream)
      return PW_STREAM_STATE_UNCONNECTED;
    auto& pw = load();
    if (!pw.stream_get_state)
      return PW_STREAM_STATE_UNCONNECTED;
    return pw.stream_get_state(m_stream, error_out);
  }

  std::uint32_t node_id() const noexcept
  {
    if (!m_stream)
      return PW_ID_ANY;
    auto& pw = load();
    return pw.stream_get_node_id ? pw.stream_get_node_id(m_stream) : PW_ID_ANY;
  }

  void disconnect() noexcept
  {
    if (!m_stream)
      return;
    auto& pw = load();
    if (!pw.stream_available || !m_ctx)
      return;
    m_ctx->with_lock([&] {
      if (pw.stream_set_active)
        pw.stream_set_active(m_stream, false);
      if (pw.stream_disconnect)
        pw.stream_disconnect(m_stream);
    });
    (void)m_ctx->synchronize();
  }

  void destroy() noexcept
  {
    if (!m_stream)
      return;
    auto& pw = load();
    if (!pw.stream_available)
    {
      m_stream = nullptr;
      return;
    }
    if (m_ctx)
    {
      disconnect();
      m_ctx->with_lock([&] {
        if (pw.stream_destroy)
          pw.stream_destroy(m_stream);
        m_stream = nullptr;
      });
    }
    else
    {
      if (pw.stream_destroy)
        pw.stream_destroy(m_stream);
      m_stream = nullptr;
    }
  }

  // RT-safe functions
  pw_buffer* dequeue_buffer() noexcept
  {
    if (!m_stream)
      return nullptr;
    auto& pw = load();
    return pw.stream_dequeue_buffer ? pw.stream_dequeue_buffer(m_stream) : nullptr;
  }

  int queue_buffer(pw_buffer* buf) noexcept
  {
    if (!m_stream || !buf)
      return -EINVAL;
    auto& pw = load();
    return pw.stream_queue_buffer ? pw.stream_queue_buffer(m_stream, buf) : -EINVAL;
  }

  int trigger_process() noexcept
  {
    if (!m_stream)
      return -ENOTCONN;
    auto& pw = load();
    return pw.stream_trigger_process ? pw.stream_trigger_process(m_stream) : -ENOTSUP;
  }

  pw_stream* handle() noexcept { return m_stream; }
  const config& cfg() const noexcept { return m_cfg; }

  void queue_buffer_from_any_thread(pw_buffer* buf, bool block = true) noexcept
  {
    if (!m_stream || !buf || !m_ctx)
      return;
    auto& pw = load();
    if (!pw.stream_queue_buffer)
      return;

    struct payload
    {
      pw_stream* stream;
      pw_buffer* buf;
      decltype(pw.stream_queue_buffer) qb;
    };

    if (block)
    {
      m_ctx->invoke_sync([&] { pw.stream_queue_buffer(m_stream, buf); });
    }
    else
    {
      m_ctx->invoke_async([s = m_stream, buf, qb = pw.stream_queue_buffer] { qb(s, buf); });
    }
  }

private:
  pw_properties* build_stream_props(const api& pw) const
  {
    auto* props = pw.properties_new(nullptr, nullptr);
    if (!props)
      return nullptr;
    pw.properties_set(props, PW_KEY_NODE_NAME, m_cfg.name.c_str());
    for (const auto& [k, v] : m_cfg.properties)
      pw.properties_set(props, k.c_str(), v.c_str());
    return props;
  }

  std::shared_ptr<context> m_ctx;
  config m_cfg;
  pw_stream* m_stream{};
};

}
