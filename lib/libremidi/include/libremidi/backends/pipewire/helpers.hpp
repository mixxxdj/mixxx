#pragma once

#include <libremidi/backends/linux/pipewire/context.hpp>
#include <libremidi/backends/linux/pipewire/filter.hpp>
#include <libremidi/backends/linux/pipewire/loader.hpp>
#include <libremidi/backends/linux/pipewire/types.hpp>
#include <libremidi/backends/pipewire/config.hpp>
#include <libremidi/detail/midi_in.hpp>

#include <pipewire/keys.h>
#include <spa/param/param.h>
#include <spa/param/buffers.h>
#include <spa/pod/builder.h>
#include <spa/utils/defs.h>

#include <cassert>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>

#if defined(__clang__)
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wsign-compare"
#endif

NAMESPACE_LIBREMIDI
{
struct pipewire_helpers
{
  using context_t = libremidi::pipewire::context;
  using filter_t = libremidi::pipewire::filter;
  using filter_config_t = libremidi::pipewire::filter_config;
  using port_token_t = libremidi::pipewire::port_token;

  const libremidi::pipewire::api& pw = libremidi::pipewire::load();

  std::shared_ptr<context_t> ctx;
  std::unique_ptr<filter_t> flt;
  port_token_t port{};
  pw_proxy* link{};

  libremidi::pipewire::subscription sub_added;
  libremidi::pipewire::subscription sub_removed;

  // on_port_removed only delivers the id; cache the full info so the
  // user-facing callback gets what it expects. Loop-thread only.
  std::unordered_map<std::uint32_t, libremidi::pipewire::port_info> port_cache;

  template <typename Self>
  stdx::error create_context(Self& self)
  {
    if (this->ctx)
      return stdx::error{};

    auto& configuration = self.configuration;

    if (configuration.core)
    {
      if (configuration.thread_loop)
      {
        this->ctx = context_t::borrow(configuration.thread_loop, configuration.core);
      }
      else if (configuration.main_loop)
      {
        this->ctx = context_t::borrow(configuration.main_loop, configuration.core);
      }
      else
      {
        return std::errc::invalid_argument;
      }
    }
    else if (configuration.thread_loop || configuration.main_loop)
    {
      return std::errc::invalid_argument;
    }
    else
    {
      this->ctx = libremidi::pipewire::shared_context();
    }

    if (!this->ctx || !this->ctx->ok())
      return std::errc::connection_refused;

    return stdx::error{};
  }

  void destroy_context()
  {
    sub_added.reset();
    sub_removed.reset();
    port_cache.clear();
    this->ctx.reset();
  }

  template <typename Self>
  stdx::error create_filter(Self& self)
  {
    if (this->flt)
      return stdx::error{};
    assert(this->ctx);

    static constexpr pw_filter_events filter_events = {
        .version = PW_VERSION_FILTER_EVENTS,
        .destroy = nullptr,
        .state_changed = nullptr,
        .io_changed = nullptr,
        .param_changed = nullptr,
        .add_buffer = nullptr,
        .remove_buffer = nullptr,
        .process =
            +[](void* data, struct spa_io_position* position) -> void {
          auto& s = *static_cast<Self*>(data);
          s.process(position);
        },
        .drained = nullptr,
#if PW_VERSION_FILTER_EVENTS >= 1
        .command = nullptr,
#endif
    };

    filter_config_t cfg;
    cfg.name = self.configuration.client_name;
    cfg.media_type = "Midi";
    cfg.media_category = "Filter";
    cfg.media_role = "DSP";
    cfg.always_process = true;
    cfg.pause_on_idle = false;
    cfg.suspend_on_idle = false;
    cfg.lock_rate = true;
    cfg.rt_process = true;

    this->flt = std::make_unique<filter_t>(
        this->ctx, std::move(cfg), filter_events, static_cast<void*>(&self));
    if (!this->flt->ok())
    {
      this->flt.reset();
      return std::errc::connection_refused;
    }

    if (int rc = this->flt->start(); rc < 0)
    {
      this->flt.reset();
      return std::errc::connection_refused;
    }

    return stdx::error{};
  }

  template <typename Self>
  void destroy_filter(Self&)
  {
    if (!this->flt)
      return;
    if (this->port.valid())
    {
      this->flt->remove_port(this->port);
      this->port = {};
    }
    this->flt.reset();
  }

  template <typename Self>
  stdx::error create_local_port(
      Self& self, std::string_view portName, spa_direction direction, const char* format)
  {
    assert(this->flt);
    if (this->port.valid())
      return stdx::error{};

    if (portName.empty())
      portName = direction == SPA_DIRECTION_INPUT ? "i" : "o";

    this->port = this->flt->add_port(direction, portName, format);
    if (!this->port.valid())
    {
      self.libremidi_handle_error(self.configuration, "error creating port");
      return std::errc::invalid_argument;
    }

    // Wait for the daemon to assign our node an id.
    this->flt->synchronize_node();
    return stdx::error{};
  }

  void set_port_buffer(std::int64_t bytes)
  {
    if (!this->flt || !this->port.valid())
      return;

    std::uint8_t buffer[1024];
    spa_pod_builder builder;
    spa_pod_builder_init(&builder, buffer, sizeof(buffer));

    // clang-format off
    const spa_pod* params[1] = {
      (spa_pod*) spa_pod_builder_add_object(
        &builder,
        SPA_TYPE_OBJECT_ParamBuffers, SPA_PARAM_Buffers,
        SPA_PARAM_BUFFERS_buffers, SPA_POD_CHOICE_RANGE_Int(1, 1, 32),
        SPA_PARAM_BUFFERS_blocks,   SPA_POD_Int(1),
        SPA_PARAM_BUFFERS_size,
          SPA_POD_CHOICE_RANGE_Int(static_cast<int>(bytes), 4096, INT32_MAX),
        SPA_PARAM_BUFFERS_stride,   SPA_POD_Int(1)
      )
    };
    // clang-format on

    this->flt->update_params(this->port, params, 1);
  }

  static bool port_matches(
      const libremidi::pipewire::port_info& port, libremidi::pipewire::media_class kind) noexcept
  {
    if (kind == libremidi::pipewire::media_class::ump)
      return port.kind == libremidi::pipewire::media_class::ump;
    return libremidi::pipewire::is_midi_like(port.kind);
  }

  template <libremidi::API Api>
  void add_callbacks(libremidi::pipewire::media_class kind, const observer_configuration& conf)
  {
    assert(this->ctx);

    sub_added = this->ctx->on_port_added(
        [this, kind, &conf](const libremidi::pipewire::port_info& port) {
          if (!port_matches(port, kind))
            return;
          this->port_cache[port.id] = port;

          bool unfiltered = conf.track_any;
          unfiltered |= (port.physical && conf.track_hardware);
          unfiltered |= (!port.physical && conf.track_virtual);
          if (!unfiltered)
            return;
          if (port.direction == SPA_DIRECTION_INPUT)
          {
            if (conf.output_added)
              conf.output_added(to_port_info<SPA_DIRECTION_INPUT, Api>(port));
          }
          else
          {
            if (conf.input_added)
              conf.input_added(to_port_info<SPA_DIRECTION_OUTPUT, Api>(port));
          }
        });

    sub_removed = this->ctx->on_port_removed(
        [this, kind, &conf](std::uint32_t port_id) {
          auto it = this->port_cache.find(port_id);
          if (it == this->port_cache.end())
            return;
          const auto port = it->second;
          this->port_cache.erase(it);

          if (!port_matches(port, kind))
            return;
          bool unfiltered = conf.track_any;
          unfiltered |= (port.physical && conf.track_hardware);
          unfiltered |= (!port.physical && conf.track_virtual);
          if (!unfiltered)
            return;
          if (port.direction == SPA_DIRECTION_INPUT)
          {
            if (conf.output_removed)
              conf.output_removed(to_port_info<SPA_DIRECTION_INPUT, Api>(port));
          }
          else
          {
            if (conf.input_removed)
              conf.input_removed(to_port_info<SPA_DIRECTION_OUTPUT, Api>(port));
          }
        });
  }

  // Shared context owns the thread loop; these are call-site shims.
  void start_thread() noexcept { }
  void stop_thread() noexcept { }

  stdx::error do_close_port()
  {
    if (!this->flt || !this->port.valid())
      return stdx::error{};
    unlink_ports();
    this->flt->remove_port(this->port);
    this->port = {};
    return stdx::error{};
  }

  stdx::error rename_port(std::string_view port_name)
  {
    if (!this->flt || !this->port.valid() || !this->ctx)
      return std::errc::not_connected;

    spa_dict_item items[1] = {
        SPA_DICT_ITEM_INIT(PW_KEY_PORT_NAME, port_name.data()),
    };
    auto properties = SPA_DICT_INIT(items, 1);
    this->ctx->with_lock([&] {
      if (pw.filter_update_properties)
        pw.filter_update_properties(this->flt->handle(), this->port.opaque, &properties);
    });
    return stdx::error{};
  }

  void unlink_ports()
  {
    if (link && this->ctx)
    {
      libremidi::pipewire::unlink_ports(*this->ctx, link);
      link = nullptr;
    }
  }

  // Polls because the daemon may not have echoed our node + ports
  // back yet; synchronize() drives it forward. Bounded for safety.
  stdx::error link_ports(auto& self, const input_port& in_port)
  {
    if (!this->flt || !this->ctx)
      return std::errc::not_connected;

    this->flt->synchronize_node();
    const auto this_node_id = this->flt->node_id();
    if (this_node_id == PW_ID_ANY)
      return std::errc::invalid_argument;

    libremidi::pipewire::port_info our_port{};
    bool found = false;
    for (int i = 0; i < 200 && !found; ++i)
    {
      auto snap = this->ctx->snapshot();
      for (const auto& n : snap.nodes)
      {
        if (n.id == this_node_id && !n.inputs.empty())
        {
          our_port = n.inputs.front();
          found = true;
          break;
        }
      }
      if (!found)
        this->ctx->synchronize();
    }
    if (!found)
      return std::errc::no_link;

    link = libremidi::pipewire::link_ports(*this->ctx, in_port.port, our_port.id);
    if (!link)
    {
      self.libremidi_handle_error(
          self.configuration,
          "could not connect to port: " + in_port.port_name + " -> " + our_port.port_name);
      return std::errc::no_link;
    }
    return stdx::error{};
  }

  stdx::error link_ports(auto& self, const output_port& out_port)
  {
    if (!this->flt || !this->ctx)
      return std::errc::not_connected;

    this->flt->synchronize_node();
    const auto this_node_id = this->flt->node_id();
    if (this_node_id == PW_ID_ANY)
      return std::errc::invalid_argument;

    libremidi::pipewire::port_info our_port{};
    bool found = false;
    for (int i = 0; i < 200 && !found; ++i)
    {
      auto snap = this->ctx->snapshot();
      for (const auto& n : snap.nodes)
      {
        if (n.id == this_node_id && !n.outputs.empty())
        {
          our_port = n.outputs.front();
          found = true;
          break;
        }
      }
      if (!found)
        this->ctx->synchronize();
    }
    if (!found)
      return std::errc::no_link;

    link = libremidi::pipewire::link_ports(*this->ctx, our_port.id, out_port.port);
    if (!link)
    {
      self.libremidi_handle_error(
          self.configuration,
          "could not connect to port: " + our_port.port_name + " -> " + out_port.port_name);
      return std::errc::no_link;
    }
    return stdx::error{};
  }

  template <spa_direction Direction, libremidi::API Api>
  static auto to_port_info(const libremidi::pipewire::port_info& port)
      -> std::conditional_t<Direction == SPA_DIRECTION_OUTPUT, input_port, output_port>
  {
    std::string device_name, port_name;
    auto name_colon = port.port_alias.find(':');
    if (name_colon != std::string::npos)
    {
      device_name = port.port_alias.substr(0, name_colon);
      port_name = port.port_alias.substr(name_colon + 1);
    }
    else
    {
      port_name = port.port_alias;
    }

    return {{
        .api = Api,
        .client = 0,
        .port = port.id,
        .manufacturer = "",
        .device_name = device_name,
        .port_name = port.port_name,
        .display_name = port_name,
    }};
  }

  // The Direction template parameter is in pipewire's sense, so a
  // user-facing input port carries Direction=OUTPUT.
  template <spa_direction Direction, libremidi::API Api>
  static auto get_ports(
      libremidi::pipewire::media_class kind, const observer_configuration& conf,
      const context_t& ctx) noexcept
      -> std::vector<
          std::conditional_t<Direction == SPA_DIRECTION_OUTPUT, input_port, output_port>>
  {
    std::vector<std::conditional_t<Direction == SPA_DIRECTION_OUTPUT, input_port, output_port>>
        ret;

    const auto snap = ctx.snapshot();
    for (const auto& node : snap.nodes)
    {
      const auto& bucket = (Direction == SPA_DIRECTION_INPUT ? node.inputs : node.outputs);
      for (const auto& port : bucket)
      {
        if (!port_matches(port, kind))
          continue;
        bool unfiltered = conf.track_any;
        unfiltered |= (port.physical && conf.track_hardware);
        unfiltered |= (!port.physical && conf.track_virtual);
        if (unfiltered)
          ret.push_back(to_port_info<Direction, Api>(port));
      }
    }
    return ret;
  }
};
}

#if defined(__clang__)
  #pragma clang diagnostic pop
#endif
