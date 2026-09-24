#pragma once
#include <libremidi/backends/linux/pipewire/context.hpp>
#include <libremidi/backends/linux/pipewire/loader.hpp>
#include <libremidi/config.hpp>

#include <pipewire/filter.h>
#include <pipewire/keys.h>
#include <pipewire/properties.h>

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>

namespace libremidi::pipewire
{

struct filter_config
{
  std::string name;
  std::string description;
  std::string media_type{"Audio"};
  std::string media_category{"Filter"};

  std::string media_role{"DSP"};

  // Audio: "32 bit float mono audio"; MIDI 1.0: "8 bit raw midi";
  // MIDI 2.0: "32 bit raw UMP".
  std::string format_dsp;

  int buffer_size{0};
  int rate{0};

  bool force_quantum{false};
  bool lock_quantum{false};
  bool force_rate{false};
  bool lock_rate{false};

  bool always_process{true};
  bool pause_on_idle{false};
  bool suspend_on_idle{false};

  bool rt_process{true};
  std::uint32_t extra_flags{0};
};

struct port_token
{
  void* opaque{};
  bool valid() const noexcept { return opaque != nullptr; }
};

class filter
{
public:
  using config = filter_config;

  filter(std::shared_ptr<context> ctx, config cfg, const pw_filter_events& events, void* user_data)
      : m_ctx{std::move(ctx)}
      , m_cfg{std::move(cfg)}
  {
    if (!m_ctx || !m_ctx->ok())
      return;
    auto& pw = load();
    if (!pw.filter_available)
      return;

    m_ctx->with_lock([&] {
      auto* props = build_filter_props(pw);
      if (!props)
        return;
      // props ownership taken by pw_filter_new_simple.
      m_filter = pw.filter_new_simple(
          m_ctx->bare_loop(), m_cfg.name.c_str(), props, &events, user_data);
    });
  }

  ~filter() { stop(); }

  filter(const filter&) = delete;
  filter& operator=(const filter&) = delete;
  filter(filter&&) = delete;
  filter& operator=(filter&&) = delete;

  bool ok() const noexcept { return m_filter != nullptr; }

  port_token add_port(
      pw_direction direction, std::string_view port_name, std::string_view format_dsp = {},
      pw_filter_port_flags port_flags = PW_FILTER_PORT_FLAG_MAP_BUFFERS,
      const spa_pod* const* params = nullptr, std::uint32_t n_params = 0)
  {
    if (!ok())
      return {};
    auto& pw = load();
    void* result = nullptr;
    m_ctx->with_lock([&] {
      auto* props = pw.properties_new(
          PW_KEY_FORMAT_DSP,
          format_dsp.empty() ? m_cfg.format_dsp.c_str() : std::string{format_dsp}.c_str(),
          PW_KEY_PORT_NAME, std::string{port_name}.c_str(), nullptr);
      if (!props)
        return;
      // props ownership taken by pw_filter_add_port.
      result = pw.filter_add_port(
          m_filter, direction, port_flags, /*port_data_size=*/0, props,
          const_cast<const spa_pod**>(params), n_params);
    });
    return port_token{result};
  }

  void remove_port(port_token token)
  {
    if (!ok() || !token.valid())
      return;
    auto& pw = load();
    m_ctx->with_lock([&] {
      if (pw.filter_remove_port)
        pw.filter_remove_port(token.opaque);
    });
  }

  int start()
  {
    if (!ok())
      return -ENOTCONN;
    auto& pw = load();
    int rc = 0;
    m_ctx->with_lock([&] {
      pw_filter_flags flags = static_cast<pw_filter_flags>(
          m_cfg.extra_flags | (m_cfg.rt_process ? PW_FILTER_FLAG_RT_PROCESS : 0));
      rc = pw.filter_connect(m_filter, flags, nullptr, 0);
    });
    return rc;
  }

  void stop() noexcept
  {
    if (!m_filter)
      return;
    auto& pw = load();
    if (!pw.filter_available)
    {
      m_filter = nullptr;
      return;
    }
    if (m_ctx)
    {
      m_ctx->with_lock([&] {
        if (pw.filter_disconnect)
          pw.filter_disconnect(m_filter);
      });
      // Let the disconnect propagate before destroy.
      (void)m_ctx->synchronize();
      m_ctx->with_lock([&] {
        if (pw.filter_destroy)
          pw.filter_destroy(m_filter);
        m_filter = nullptr;
      });
    }
    else
    {
      if (pw.filter_destroy)
        pw.filter_destroy(m_filter);
      m_filter = nullptr;
    }
  }

  std::uint32_t node_id() const noexcept
  {
    if (!m_filter)
      return PW_ID_ANY;
    auto& pw = load();
    return pw.filter_get_node_id ? pw.filter_get_node_id(m_filter) : PW_ID_ANY;
  }

  bool synchronize_node(int max_attempts = 100)
  {
    for (int i = 0; i < max_attempts; ++i)
    {
      if (node_id() != PW_ID_ANY)
        return true;
      if (!m_ctx || !m_ctx->synchronize())
        return false;
    }
    return false;
  }

  int update_params(port_token port, const spa_pod* const* params, std::uint32_t n_params)
  {
    if (!ok())
      return -ENOTCONN;
    auto& pw = load();
    int rc = 0;
    m_ctx->with_lock([&] {
      rc = pw.filter_update_params(
          m_filter, port.opaque, const_cast<const spa_pod**>(params), n_params);
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
      if (pw.filter_set_active)
        rc = pw.filter_set_active(m_filter, active);
    });
    return rc;
  }

  pw_filter* handle() noexcept { return m_filter; }
  const config& cfg() const noexcept { return m_cfg; }

  // For use in DSP thread:
  void* get_dsp_buffer(port_token port, std::uint32_t n_samples) noexcept
  {
    if (!m_filter || !port.valid())
      return nullptr;
    auto& pw = load();
    return pw.filter_get_dsp_buffer ? pw.filter_get_dsp_buffer(port.opaque, n_samples) : nullptr;
  }

  pw_buffer* dequeue_buffer(port_token port) noexcept
  {
    if (!m_filter || !port.valid())
      return nullptr;
    auto& pw = load();
    return pw.filter_dequeue_buffer ? pw.filter_dequeue_buffer(port.opaque) : nullptr;
  }

  void queue_buffer(port_token port, pw_buffer* buf) noexcept
  {
    if (!m_filter || !port.valid() || !buf)
      return;
    auto& pw = load();
    if (pw.filter_queue_buffer)
      pw.filter_queue_buffer(port.opaque, buf);
  }

private:
  pw_properties* build_filter_props(const api& pw) const
  {
    auto* props = pw.properties_new(
        PW_KEY_MEDIA_TYPE, m_cfg.media_type.c_str(), PW_KEY_MEDIA_CATEGORY,
        m_cfg.media_category.c_str(), PW_KEY_MEDIA_ROLE, m_cfg.media_role.c_str(),
        PW_KEY_MEDIA_NAME, m_cfg.name.c_str(), PW_KEY_NODE_NAME, m_cfg.name.c_str(), nullptr);
    if (!props)
      return nullptr;
    if (!m_cfg.description.empty())
      pw.properties_set(props, PW_KEY_NODE_DESCRIPTION, m_cfg.description.c_str());

    if (m_cfg.buffer_size > 0 && m_cfg.rate > 0)
    {
      char latency[32];
      std::snprintf(latency, sizeof(latency), "%d/%d", m_cfg.buffer_size, m_cfg.rate);
      pw.properties_set(props, PW_KEY_NODE_LATENCY, latency);
    }
    if (m_cfg.force_quantum && m_cfg.buffer_size > 0)
    {
      char buf[16];
      std::snprintf(buf, sizeof(buf), "%d", m_cfg.buffer_size);
      pw.properties_set(props, PW_KEY_NODE_FORCE_QUANTUM, buf);
    }
    if (m_cfg.lock_quantum)
      pw.properties_set(props, PW_KEY_NODE_LOCK_QUANTUM, "true");
    if (m_cfg.force_rate && m_cfg.rate > 0)
    {
      char buf[16];
      std::snprintf(buf, sizeof(buf), "%d", m_cfg.rate);
      pw.properties_set(props, PW_KEY_NODE_FORCE_RATE, buf);
    }
    if (m_cfg.lock_rate)
      pw.properties_set(props, PW_KEY_NODE_LOCK_RATE, "true");
    if (m_cfg.always_process)
      pw.properties_set(props, PW_KEY_NODE_ALWAYS_PROCESS, "true");
    if (!m_cfg.pause_on_idle)
      pw.properties_set(props, PW_KEY_NODE_PAUSE_ON_IDLE, "false");
    if (!m_cfg.suspend_on_idle)
      pw.properties_set(props, PW_KEY_NODE_SUSPEND_ON_IDLE, "false");
    return props;
  }

  std::shared_ptr<context> m_ctx;
  config m_cfg;
  pw_filter* m_filter{};
};

inline pw_proxy* link_ports(context& ctx, std::uint32_t out_port, std::uint32_t in_port) noexcept
{
  if (!ctx.ok())
    return nullptr;
  auto& pw = load();
  pw_proxy* result = nullptr;
  ctx.with_lock([&] {
    auto* props = pw.properties_new(
        PW_KEY_LINK_OUTPUT_PORT, std::to_string(out_port).c_str(), PW_KEY_LINK_INPUT_PORT,
        std::to_string(in_port).c_str(), nullptr);
    if (!props)
      return;
    result = reinterpret_cast<pw_proxy*>(pw_core_create_object(
        ctx.pw_core_ptr(), "link-factory", PW_TYPE_INTERFACE_Link, PW_VERSION_LINK, &props->dict,
        0));
    pw.properties_free(props);
  });
  if (result)
    (void)ctx.synchronize();
  return result;
}

// Link by node ids; pipewire activates ports as needed. Required to
// reach a suspended sink that hasn't exposed ports (matches the
// `pw-link <node> <node>` path). Release via unlink_ports.
inline pw_proxy* link_nodes(context& ctx, std::uint32_t out_node, std::uint32_t in_node) noexcept
{
  if (!ctx.ok())
    return nullptr;
  auto& pw = load();
  pw_proxy* result = nullptr;
  ctx.with_lock([&] {
    auto* props = pw.properties_new(
        PW_KEY_LINK_OUTPUT_NODE, std::to_string(out_node).c_str(), PW_KEY_LINK_INPUT_NODE,
        std::to_string(in_node).c_str(),
        nullptr);
    if (!props)
      return;
    result = reinterpret_cast<pw_proxy*>(pw_core_create_object(
        ctx.pw_core_ptr(), "link-factory", PW_TYPE_INTERFACE_Link, PW_VERSION_LINK, &props->dict,
        0));
    pw.properties_free(props);
  });
  if (result)
    (void)ctx.synchronize();
  return result;
}

inline void unlink_ports(context& ctx, pw_proxy* link) noexcept
{
  if (!link)
    return;
  auto& pw = load();
  ctx.with_lock([&] {
    if (pw.proxy_destroy)
      pw.proxy_destroy(link);
  });
}

}
