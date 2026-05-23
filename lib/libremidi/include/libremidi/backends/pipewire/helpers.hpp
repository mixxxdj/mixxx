#pragma once

#include <libremidi/backends/linux/helpers.hpp>
#include <libremidi/backends/linux/pipewire.hpp>
#include <libremidi/backends/pipewire/config.hpp>
#include <libremidi/backends/pipewire/context.hpp>
#include <libremidi/detail/memory.hpp>
#include <libremidi/detail/midi_in.hpp>
#include <libremidi/detail/semaphore.hpp>

#include <atomic>
#include <thread>

#if defined(__clang__)
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wsign-compare"
#endif

NAMESPACE_LIBREMIDI
{
struct pipewire_helpers
{
  struct port
  {
    void* data{};
  };

  // All pipewire operations have to happen in the same thread
  // - and pipewire checks that internally.
  std::jthread main_loop_thread;
  const libpipewire& pw = libpipewire::instance();
  std::shared_ptr<pipewire_instance> global_instance;
  std::shared_ptr<pipewire_context> global_context;
  std::unique_ptr<pipewire_filter> filter;
  pw_proxy* link{};

  int64_t this_instance{};

  eventfd_notifier termination_event{};
  pollfd fds[2]{};

  semaphore_pair_lock thread_lock;
  std::shared_ptr<void> canary = std::make_shared<int>();

  enum poll_state
  {
    start_poll,
    in_poll,
    not_in_poll
  };
  std::atomic<poll_state> current_state{not_in_poll};

  pipewire_helpers()
  {
    static std::atomic_int64_t instance{};
    this_instance = ++instance;

    fds[1] = termination_event;
  }

  template <typename Self>
  stdx::error create_filter(Self& self)
  {
    if (this->filter)
      return stdx::error{};

    auto& configuration = self.configuration;
    if (configuration.context && configuration.filter && configuration.set_process_func)
    {
      this->filter = std::make_unique<pipewire_filter>(this->global_context, configuration.filter);

      libremidi::pipewire_callback cbs{
          .token = this_instance,
          .callback = [&self, p = std::weak_ptr{canary}](spa_io_position* nf) -> void {
        if (auto pt = p.lock())
          self.process(nf);

        self.thread_lock.check_client_released();
      }};
      configuration.set_process_func(cbs);
    }
    else
    {
      this->filter = std::make_unique<pipewire_filter>(this->global_context);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
      static constexpr struct pw_filter_events filter_events
          = {.version = PW_VERSION_FILTER_EVENTS,
             .process = +[](void* _data, struct spa_io_position* position) -> void {
        // FIXME likely we need the thread_lock check here too
        Self& self = *static_cast<Self*>(_data);
        self.process(position);
      }};
#pragma GCC diagnostic pop

      this->filter->create_filter(self.configuration.client_name, filter_events, &self);
      return this->filter->start_filter();
    }
    return stdx::error{};
  }

  template <typename Self>
  void destroy_filter(Self& self)
  {
    assert(global_context);
    if (!global_context->owns_main_loop)
    {
      if (self.configuration.clear_process_func)
      {
        self.configuration.clear_process_func(this_instance);
      }
    }
    else
    {
      if (this->filter)
      {
        this->filter->destroy();
      }
    }

    this->filter.reset();
  }

  template <typename Self>
  stdx::error create_context(Self& self)
  {
    if (this->global_context)
      return stdx::error{};

    // Initialize PipeWire client
    auto& configuration = self.configuration;
    if (configuration.context)
    {
      this->global_context = std::make_shared<pipewire_context>(configuration.context);
    }
    else
    {
      this->global_instance = std::make_shared<pipewire_instance>();
      this->global_context = std::make_shared<pipewire_context>(this->global_instance);
    }
    if (!this->global_context->main_loop)
      return std::errc::connection_refused;

    return stdx::error{};
  }

  void destroy_context()
  {
    assert(this->global_context);
    this->global_context.reset();
    this->global_instance.reset();
  }

  void run_poll_loop()
  try
  {
    // Note: called from a std::jthread.
    assert(this->global_context);
    if (int fd = this->global_context->get_fd(); fd != -1)
    {
      fds[0] = {.fd = fd, .events = POLLIN, .revents = 0};
      current_state = poll_state::in_poll;

      for (;;)
      {
        if (int err = poll(fds, 2, -1); err < 0)
        {
          if (err == -EAGAIN)
            continue;
          else
            break;
        }

        // Check pipewire fd:
        if (fds[0].revents & POLLIN)
        {
          if (auto lp = this->global_context->lp)
          {
            int result = pw_loop_iterate(lp, 0);
            if (result < 0)
            {
              LIBREMIDI_LOG(spa_strerror(result));
            }
          }
          fds[0].revents = 0;
        }

        // Check exit fd:
        if (fds[1].revents & POLLIN)
        {
          break;
        }
      }
    }
    current_state = poll_state::not_in_poll;
  }
  catch (...)
  {
    current_state = poll_state::not_in_poll;
  }

  template <typename Self>
  stdx::error create_local_port(
      Self& self, std::string_view portName, spa_direction direction, const char* format)
  {
    assert(this->global_context);
    assert(this->filter);

    if (portName.empty())
      portName = direction == SPA_DIRECTION_INPUT ? "i" : "o";

    if (!this->filter->port)
    {
      auto ret = this->filter->create_local_port(portName.data(), direction, format);
      if (ret != stdx::error{})
      {
        self.libremidi_handle_error(self.configuration, "error creating port");
        return ret;
      }
    }

    return stdx::error{};
  }

  template <libremidi::API Api>
  void add_callbacks(std::string format, const observer_configuration& conf)
  {
    assert(global_context);
    global_context->on_port_added = [format, &conf](const pipewire_context::port_info& port) {
      if (port.format.find(format) == std::string::npos)
        return;

      bool unfiltered = conf.track_any;
      unfiltered |= (port.physical && conf.track_hardware);
      unfiltered |= (!port.physical && conf.track_virtual);
      if (unfiltered)
      {
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
      }
    };

    global_context->on_port_removed = [format, &conf](const pipewire_context::port_info& port) {
      if (port.format.find(format) == std::string::npos)
        return;

      bool unfiltered = conf.track_any;
      unfiltered |= (port.physical && conf.track_hardware);
      unfiltered |= (!port.physical && conf.track_virtual);
      if (unfiltered)
      {
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
      }
    };
  }

  void start_thread()
  {
    if (!this->global_context->owns_main_loop)
      return;

    current_state = poll_state::start_poll;
    main_loop_thread = std::jthread{[this]() { run_poll_loop(); }};
  }

  void stop_thread()
  {
    assert(this->global_context);
    if (!this->global_context->owns_main_loop)
      return;

    if (main_loop_thread.joinable() || current_state != poll_state::not_in_poll)
    {
      termination_event.notify();
      main_loop_thread.request_stop();

      termination_event.notify();
      for (int i = 0; i < 100; i++)
      {
        if (current_state == poll_state::not_in_poll)
          break;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        termination_event.notify();
      }

      if (main_loop_thread.joinable())
        main_loop_thread.join();
    }
  }

  stdx::error do_close_port()
  {
    if (!this->filter)
      return stdx::error{};
    if (!this->filter->port)
      return stdx::error{};

    if (!this->global_context->owns_main_loop)
    {
      this->canary.reset();
      this->thread_lock.prepare_release_client();
    }

    unlink_ports();
    return this->filter->remove_port();
  }

  stdx::error rename_port(std::string_view port_name)
  {
    if (this->filter)
    {
      return this->filter->rename_port(port_name);
    }
    else
    {
      return std::errc::not_connected;
    }
  }

  void unlink_ports()
  {
    if (link)
    {
      this->global_context->unlink_ports(link);
      link = nullptr;
    }
  }

  stdx::error link_ports(auto& self, const input_port& in_port)
  {
    // Wait for the pipewire server to send us back our node's info
    for (int i = 0; i < 1000; i++)
      this->filter->synchronize_node();

    auto this_node = this->filter->filter_node_id();
    auto& midi = this->global_context->current_graph.software_midi;
    auto node_it = midi.find(this_node);
    if (node_it == midi.end())
      return std::errc::invalid_argument;

    // Wait for the pipewire server to send us back our node's ports
    this->filter->synchronize_ports(node_it->second);

    if (node_it->second.inputs.empty())
      return std::errc::no_link;

    // Link ports
    const auto& p = node_it->second.inputs.front();
    link = this->global_context->link_ports(in_port.port, p.id);
    pw_loop_iterate(this->global_context->lp, 1);
    if (!link)
    {
      self.libremidi_handle_error(
          self.configuration,
          "could not connect to port: " + in_port.port_name + " -> " + p.port_name);
      return std::errc::no_link;
    }

    return stdx::error{};
  }

  stdx::error link_ports(auto& self, const output_port& out_port)
  {
    // Wait for the pipewire server to send us back our node's info
    for (int i = 0; i < 1000; i++)
      this->filter->synchronize_node();

    auto this_node = this->filter->filter_node_id();
    auto& midi = this->global_context->current_graph.software_midi;
    auto node_it = midi.find(this_node);
    if (node_it == midi.end())
    {
      return std::errc::invalid_argument;
    }

    // Wait for the pipewire server to send us back our node's ports
    this->filter->synchronize_ports(node_it->second);

    if (node_it->second.outputs.empty())
    {
      return std::errc::no_link;
    }

    // Link ports
    const auto& p = node_it->second.outputs.front();
    link = this->global_context->link_ports(p.id, out_port.port);
    pw_loop_iterate(this->global_context->lp, 1);
    if (!link)
    {
      self.libremidi_handle_error(
          self.configuration,
          "could not connect to port: " + p.port_name + " -> " + out_port.port_name);
      return std::errc::no_link;
    }

    return stdx::error{};
  }

  template <spa_direction Direction, libremidi::API Api>
  static auto to_port_info(const pipewire_context::port_info& port)
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

  // Note: keep in mind that an "input" port for us (e.g. a keyboard that goes to the computer)
  // is an "output" port from the point of view of pipewire as data will come out of it
  template <spa_direction Direction, libremidi::API Api>
  static auto get_ports(
      std::string_view format, const observer_configuration& conf,
      const pipewire_context& ctx) noexcept
      -> std::vector<
          std::conditional_t<Direction == SPA_DIRECTION_OUTPUT, input_port, output_port>>
  {
    std::vector<std::conditional_t<Direction == SPA_DIRECTION_OUTPUT, input_port, output_port>>
        ret;

    {
      std::lock_guard _{ctx.current_graph.mtx};
      if (conf.track_any || conf.track_hardware)
        for (auto& node : ctx.current_graph.physical_midi)
        {
          for (auto& port :
               (Direction == SPA_DIRECTION_INPUT ? node.second.inputs : node.second.outputs))
          {
            if (port.format.find(format) != std::string::npos)
              ret.push_back(to_port_info<Direction, Api>(port));
          }
        }

      if (conf.track_any || conf.track_virtual)
        for (auto& node : ctx.current_graph.software_midi)
        {
          for (auto& port :
               (Direction == SPA_DIRECTION_INPUT ? node.second.inputs : node.second.outputs))
          {
            if (port.format.find(format) != std::string::npos)
              ret.push_back(to_port_info<Direction, Api>(port));
          }
        }
    }

    return ret;
  }
};
}

#if defined(__clang__)
  #pragma clang diagnostic pop
#endif
