#pragma once
#include <libremidi/backends/linux/pipewire.hpp>
#include <libremidi/detail/memory.hpp>
#include <libremidi/error.hpp>

#include <pipewire/filter.h>
#include <pipewire/pipewire.h>
#include <spa/control/control.h>
#include <spa/param/props.h>
#include <spa/utils/defs.h>
#include <spa/utils/result.h>

#include <algorithm>
#include <array>
#include <atomic>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <spa/param/audio/format-utils.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
NAMESPACE_LIBREMIDI
{
template <typename K, typename V>
using hash_map = std::unordered_map<K, V>;

struct pipewire_instance
{
  const libpipewire& pw = libpipewire::instance();
  pipewire_instance()
  {
    /// Initialize the PipeWire main loop, context, etc.
    int argc = 0;
    char* argv[] = {NULL};
    char** aa = argv;
    pw.init(&argc, &aa);
  }

  ~pipewire_instance() { pw.deinit(); }
};

struct pipewire_context
{
  struct listened_port
  {
    uint32_t id{};
    pw_port* port{};
    std::unique_ptr<spa_hook> listener;
  };

  struct port_info
  {
    uint32_t id{};

    std::string format;
    std::string port_name;
    std::string port_alias;
    std::string object_path;
    std::string node_id;
    std::string port_id;

    bool physical{};
    bool terminal{};
    bool monitor{};
    pw_direction direction{};
  };

  struct node
  {
    std::vector<port_info> inputs;
    std::vector<port_info> outputs;
  };

  struct graph
  {
    mutable std::mutex mtx;
    libremidi::hash_map<uint64_t, node> physical_audio;
    libremidi::hash_map<uint64_t, node> physical_midi;
    libremidi::hash_map<uint64_t, node> software_audio;
    libremidi::hash_map<uint64_t, node> software_midi;
    libremidi::hash_map<uint64_t, port_info> port_cache;

    void for_each_port(auto func)
    {
      for (auto& map : {physical_audio, physical_midi, software_audio, software_midi})
      {
        for (auto& [id, node] : map)
        {
          for (auto& port : node.inputs)
            func(port);
          for (auto& port : node.outputs)
            func(port);
        }
      }
    }

    void remove_port(uint32_t id)
    {
      port_cache.erase(id);
      for (auto map : {&physical_audio, &physical_midi, &software_audio, &software_midi})
      {
        for (auto& [_, node] : *map)
        {
          std::erase_if(node.inputs, [id](const port_info& p) { return p.id == id; });
          std::erase_if(node.outputs, [id](const port_info& p) { return p.id == id; });
        }
      }
    }
  } current_graph;

  explicit pipewire_context(pw_main_loop* inst)
      : main_loop{inst}
      , owns_main_loop{false}
  {
    assert(main_loop);

    initialize();
  }

  explicit pipewire_context(std::shared_ptr<pipewire_instance> inst)
      : global_instance{std::move(inst)}
      , owns_main_loop{true}
  {
    this->main_loop = pw.main_loop_new(nullptr);
    if (!this->main_loop)
    {
      // libremidi::logger().libremidi_handle_error("main_loop_new failed!");
      return;
    }
    initialize();
  }

  void initialize()
  {
    this->lp = pw.main_loop_get_loop(this->main_loop);
    if (!lp)
    {
      // libremidi::logger().libremidi_handle_error("main_loop_get_loop failed!");
      return;
    }

    this->context = pw.context_new(lp, nullptr, 0);
    if (!this->context)
    {
      // libremidi::logger().libremidi_handle_error("context_new failed!");
      return;
    }

    this->core = pw.context_connect(this->context, nullptr, 0);
    if (!this->core)
    {
      // libremidi::logger().libremidi_handle_error("context_connect failed!");
      return;
    }

    this->registry = pw_core_get_registry(this->core, PW_VERSION_REGISTRY, 0);
    if (!this->registry)
    {
      // libremidi::logger().libremidi_handle_error("core_get_registry failed!");
      return;
    }

    initialize_observation();

    synchronize();

    // Add a manual 1ms event loop iteration at the end of
    // ctor to ensure synchronous clients will still see the ports
    pw_loop_iterate(this->lp, 1);
  }

  void initialize_observation()
  {
    // Register a listener which will listen on when ports are added / removed
    spa_zero(registry_listener);

    static constexpr const struct pw_registry_events registry_events = {
        .version = PW_VERSION_REGISTRY_EVENTS,
        .global =
            [](void* object, uint32_t id, uint32_t /*permissions*/, const char* type,
               uint32_t /*version*/, const struct spa_dict* /*props*/) {
      pipewire_context& self = *(pipewire_context*)object;
      if (strcmp(type, PW_TYPE_INTERFACE_Port) == 0)
        self.register_port(id, type);
    },
        .global_remove =
            [](void* object, uint32_t id) {
      pipewire_context& self = *(pipewire_context*)object;
      self.unregister_port(id);
    },
    };

    // Start listening
    pw_registry_add_listener(this->registry, &this->registry_listener, &registry_events, this);
  }

  void register_port(uint32_t id, const char* type)
  {
    auto port = (pw_port*)pw_registry_bind(registry, id, type, PW_VERSION_PORT, 0);
    port_listener.push_back({id, port, std::make_unique<spa_hook>()});
    auto& l = port_listener.back();

    static constexpr const struct pw_port_events port_events = {
        .version = PW_VERSION_PORT_EVENTS,
        .info
        = [](void* object,
             const pw_port_info* info) { ((pipewire_context*)object)->update_port_info(info); },
    };
    pw_port_add_listener(l.port, l.listener.get(), &port_events, this);
  }

  void unregister_port(uint32_t id)
  {
    // When a port is removed:
    // Notify
    std::unique_lock _{current_graph.mtx, std::defer_lock};
    if (on_port_removed)
    {
      _.lock();
      if (auto it = current_graph.port_cache.find(id); it != current_graph.port_cache.end())
      {
        auto copy = it->second;
        _.unlock();
        on_port_removed(copy);
      }
      else
      {
        _.unlock();
      }
    }

    // Remove from the graph
    {
      _.lock();
      current_graph.remove_port(id);
      _.unlock();
    }

    // Remove from the listeners
    auto it
        = std::find_if(port_listener.begin(), port_listener.end(), [&](const listened_port& l) {
      return l.id == id;
    });
    if (it != port_listener.end())
    {
      pw.proxy_destroy((pw_proxy*)it->port);
      port_listener.erase(it);
    }
  }

  void synchronize()
  {
    pending = 0;
    done = 0;

    if (!core)
      return;

    spa_hook core_listener;

    static constexpr struct pw_core_events core_events = {
        .version = PW_VERSION_CORE_EVENTS,
        .done =
            [](void* object, uint32_t id, int seq) {
      auto& self = *(pipewire_context*)object;
      if (id == PW_ID_CORE && seq == self.pending)
      {
        self.done = 1;
        libpipewire::instance().main_loop_quit(self.main_loop);
      }
    },
    };

    spa_zero(core_listener);
    pw_core_add_listener(core, &core_listener, &core_events, this);

    pending = pw_core_sync(core, PW_ID_CORE, 0);
    while (!done)
    {
      pw.main_loop_run(this->main_loop);
    }
    spa_hook_remove(&core_listener);
  }

  [[nodiscard]] pw_proxy* link_ports(uint64_t out_port, uint64_t in_port)
  {
    auto props = pw.properties_new(
        PW_KEY_LINK_OUTPUT_PORT, std::to_string(out_port).c_str(), PW_KEY_LINK_INPUT_PORT,
        std::to_string(in_port).c_str(), nullptr);

    auto proxy = (pw_proxy*)pw_core_create_object(
        this->core, "link-factory", PW_TYPE_INTERFACE_Link, PW_VERSION_LINK, &props->dict, 0);

    if (!proxy)
    {
      pw.properties_free(props);
      return nullptr;
    }

    synchronize();
    pw.properties_free(props);
    return proxy;
  }

  void unlink_ports(pw_proxy* link) { pw.proxy_destroy(link); }

  void update_port_info(const pw_port_info* info)
  {
    const spa_dict_item* item{};

    port_info p;
    p.id = info->id;

    spa_dict_for_each(item, info->props)
    {
      std::string_view k{item->key}, v{item->value};
      if (k == "format.dsp")
        p.format = v;
      else if (k == "port.name")
        p.port_name = v;
      else if (k == "port.alias")
        p.port_alias = v;
      else if (k == "object.path")
        p.object_path = v;
      else if (k == "port.id")
        p.port_id = v;
      else if (k == "node.id")
        p.node_id = v;
      else if (k == "port.physical" && v == "true")
        p.physical = true;
      else if (k == "port.terminal" && v == "true")
        p.terminal = true;
      else if (k == "port.monitor" && v == "true")
        p.monitor = true;
      else if (k == "port.direction")
      {
        if (v == "out")
        {
          p.direction = pw_direction::SPA_DIRECTION_OUTPUT;
        }
        else
        {
          p.direction = pw_direction::SPA_DIRECTION_INPUT;
        }
      }
    }

    if (p.node_id.empty())
      return;

    const auto nid = std::stoul(p.node_id);
    auto get_node = [&]() -> node* {
      if (p.physical)
      {
        if (p.format.find("audio") != p.format.npos)
          return &this->current_graph.physical_audio[nid];
        else if (p.format.find("midi") != p.format.npos)
          return &this->current_graph.physical_midi[nid];
        else if (p.format.find("UMP") != p.format.npos)
          return &this->current_graph.physical_midi[nid];
      }
      else
      {
        if (p.format.find("audio") != p.format.npos)
          return &this->current_graph.software_audio[nid];
        else if (p.format.find("midi") != p.format.npos)
          return &this->current_graph.software_midi[nid];
        else if (p.format.find("UMP") != p.format.npos)
          return &this->current_graph.software_midi[nid];
      }
      return nullptr;
    };

    {
      std::lock_guard _{current_graph.mtx};
      current_graph.port_cache[p.id] = p;
      if (auto node = get_node())
      {
        if (p.direction == pw_direction::SPA_DIRECTION_OUTPUT)
          node->outputs.push_back(p);
        else
          node->inputs.push_back(p);
      }
    }

    if (on_port_added)
      on_port_added(p);
  }

  int get_fd() const noexcept
  {
    if (!this->lp)
      return -1;

    auto spa_callbacks = this->lp->control->iface.cb;
    auto spa_loop_methods = (const spa_loop_control_methods*)spa_callbacks.funcs;
    if (spa_loop_methods->get_fd)
      return spa_loop_methods->get_fd(spa_callbacks.data);
    else
      return -1;
  }

  ~pipewire_context()
  {
    if (this->registry)
      pw.proxy_destroy((pw_proxy*)this->registry);
    for (auto& [id, p, l] : this->port_listener)
      if (l)
        pw.proxy_destroy((pw_proxy*)p);
    if (this->core)
      pw.core_disconnect(this->core);
    if (this->context)
      pw.context_destroy(this->context);
    if (owns_main_loop && this->main_loop)
      pw.main_loop_destroy(this->main_loop);
  }

  friend struct pipewire_filter;
  const libpipewire& pw = libpipewire::instance();
  std::shared_ptr<pipewire_instance> global_instance;

  pw_main_loop* main_loop{};
  pw_loop* lp{};

  pw_context* context{};
  pw_core* core{};

  pw_registry* registry{};
  spa_hook registry_listener{};

  std::function<void(const port_info&)> on_port_added;
  std::function<void(const port_info&)> on_port_removed;

  std::vector<listened_port> port_listener{};

  std::atomic<int> pending{};
  std::atomic<int> done{};
  bool owns_main_loop{true};
  int sync{};
};

struct pipewire_filter
{
  const libpipewire& pw = libpipewire::instance();
  std::shared_ptr<pipewire_context> loop{};
  pw_filter* filter{};
  std::vector<pw_proxy*> links{};

  struct port
  {
    void* data;
  }* port{};

  explicit pipewire_filter(std::shared_ptr<pipewire_context> loop)
      : loop{std::move(loop)}
  {
  }

  explicit pipewire_filter(std::shared_ptr<pipewire_context> loop, pw_filter* filter)
      : loop{std::move(loop)}
      , filter{filter}
  {
  }

  void create_filter(std::string_view filter_name, const pw_filter_events& events, void* context)
  {
    assert(!filter);

    auto& pw = libpipewire::instance();
    // clang-format off
    this->filter = pw.filter_new_simple(
        loop->lp,
        filter_name.data(),
        pw.properties_new(
            PW_KEY_MEDIA_TYPE, "Midi",
            PW_KEY_MEDIA_CATEGORY, "Filter",
            PW_KEY_MEDIA_ROLE, "DSP",
            PW_KEY_MEDIA_NAME, "",
#if defined(PW_KEY_NODE_LOCK_RATE)
            PW_KEY_NODE_LOCK_RATE, "true",
#endif
            PW_KEY_NODE_ALWAYS_PROCESS, "true",
            PW_KEY_NODE_PAUSE_ON_IDLE, "false",
#if defined(PW_KEY_NODE_SUSPEND_ON_IDLE)
            PW_KEY_NODE_SUSPEND_ON_IDLE, "false",
#endif
            nullptr),
        &events,
        context);
    // clang-format on
    assert(filter);
  }

  void destroy()
  {
    if (this->filter)
      pw.filter_destroy(this->filter);
  }

  stdx::error
  create_local_port(std::string_view port_name, spa_direction direction, const char* format)
  {
    // clang-format off
    this->port = (struct port*)pw.filter_add_port(
        this->filter,
        direction,
        PW_FILTER_PORT_FLAG_MAP_BUFFERS,
        sizeof(struct port),
        pw.properties_new(
            PW_KEY_FORMAT_DSP, format,
            PW_KEY_PORT_NAME, port_name.data(),
            nullptr),
        nullptr, 0);
    // clang-format on
    if (!port)
      return std::errc::invalid_argument;
    return stdx::error{};
  }

  void set_port_buffer(int64_t bytes)
  {
    uint8_t buffer[1024];
    struct spa_pod_builder builder;
    spa_pod_builder_init(&builder, buffer, sizeof(buffer));

    // clang-format off
    const struct spa_pod* params[1] = {
      (spa_pod*) spa_pod_builder_add_object(
        &builder,
        SPA_TYPE_OBJECT_ParamBuffers, SPA_PARAM_Buffers,
        SPA_PARAM_BUFFERS_buffers, SPA_POD_CHOICE_RANGE_Int(1, 1, 32),
        SPA_PARAM_BUFFERS_blocks, SPA_POD_Int(1),
        SPA_PARAM_BUFFERS_size, SPA_POD_CHOICE_RANGE_Int(bytes, 4096, INT32_MAX),
        SPA_PARAM_BUFFERS_stride, SPA_POD_Int(1)
      )
    };
    // clang-format on

    pw.filter_update_params(this->filter, this->port, params, 1);
  }

  stdx::error remove_port()
  {
    assert(this->port);
    int ret = pw.filter_remove_port(this->port);
    this->port = nullptr;
    return from_errc(ret);
  }

  stdx::error rename_port(std::string_view port_name)
  {
    if (this->port)
    {
      spa_dict_item items[1] = {
          SPA_DICT_ITEM_INIT(PW_KEY_PORT_NAME, port_name.data()),
      };

      auto properties = SPA_DICT_INIT(items, 1);
      int ret = pw.filter_update_properties(this->filter, this->port, &properties);
      return from_errc(ret);
    }
    else
    {
      return std::errc::not_connected;
    }
  }

  [[nodiscard]] stdx::error start_filter()
  {
    if (int ret = pw.filter_connect(this->filter, PW_FILTER_FLAG_RT_PROCESS, NULL, 0); ret < 0)
    {
      return from_errc(ret);
    }
    else
    {
      return stdx::error{};
    }
  }

  uint32_t filter_node_id() { return this->loop->pw.filter_get_node_id(this->filter); }

  void synchronize_node()
  {
    this->loop->synchronize();
    int k = 0;
    auto node_id = filter_node_id();
    while (node_id == 4294967295)
    {
      this->loop->synchronize();
      node_id = filter_node_id();

      if (k++; k > 100)
        return;
    }
  }
  void synchronize_ports(const pipewire_context::node& this_node)
  {
    // Leave some time to resolve the ports
    int k = 0;
    const auto num_local_ins = 1;
    const auto num_local_outs = 0;
    while (this_node.inputs.size() < num_local_ins || this_node.outputs.size() < num_local_outs)
    {
      this->loop->synchronize();
      if (k++; k > 100)
        return;
    }
  }
};
}

#pragma GCC diagnostic pop
