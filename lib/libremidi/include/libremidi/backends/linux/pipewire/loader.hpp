#pragma once

#include <libremidi/backends/linux/dylib_loader.hpp>
#include <libremidi/config.hpp>

#include <pipewire/context.h>
#include <pipewire/core.h>
#include <pipewire/filter.h>
#include <pipewire/pipewire.h>
#include <pipewire/properties.h>
#include <pipewire/proxy.h>
#include <pipewire/stream.h>

#include <cassert>

#include <pipewire/impl-module.h>
#include <pipewire/main-loop.h>
#include <pipewire/thread-loop.h>

namespace libremidi::pipewire
{
class api
{
public:
  decltype(&::pw_init) init{};
  decltype(&::pw_deinit) deinit{};
  decltype(&::pw_get_library_version) get_library_version{};

  decltype(&::pw_main_loop_new) main_loop_new{};
  decltype(&::pw_main_loop_destroy) main_loop_destroy{};
  decltype(&::pw_main_loop_run) main_loop_run{};
  decltype(&::pw_main_loop_quit) main_loop_quit{};
  decltype(&::pw_main_loop_get_loop) main_loop_get_loop{};

  decltype(&::pw_thread_loop_new) thread_loop_new{};
  decltype(&::pw_thread_loop_new_full) thread_loop_new_full{};
  decltype(&::pw_thread_loop_destroy) thread_loop_destroy{};
  decltype(&::pw_thread_loop_start) thread_loop_start{};
  decltype(&::pw_thread_loop_stop) thread_loop_stop{};
  decltype(&::pw_thread_loop_get_loop) thread_loop_get_loop{};
  decltype(&::pw_thread_loop_lock) thread_loop_lock{};
  decltype(&::pw_thread_loop_unlock) thread_loop_unlock{};
  decltype(&::pw_thread_loop_signal) thread_loop_signal{};
  decltype(&::pw_thread_loop_wait) thread_loop_wait{};
  decltype(&::pw_thread_loop_timed_wait) thread_loop_timed_wait{};
  decltype(&::pw_thread_loop_in_thread) thread_loop_in_thread{};
  decltype(&::pw_thread_loop_add_listener) thread_loop_add_listener{};
  decltype(&::pw_thread_loop_accept) thread_loop_accept{};

  decltype(&::pw_context_new) context_new{};
  decltype(&::pw_context_destroy) context_destroy{};
  decltype(&::pw_context_connect) context_connect{};
  decltype(&::pw_context_connect_fd) context_connect_fd{};
  decltype(&::pw_context_load_module) context_load_module{};
  decltype(&::pw_core_disconnect) core_disconnect{};

  decltype(&::pw_proxy_add_listener) proxy_add_listener{};
  decltype(&::pw_proxy_destroy) proxy_destroy{};

  decltype(&::pw_properties_new) properties_new{};
  decltype(&::pw_properties_new_dict) properties_new_dict{};
  decltype(&::pw_properties_set) properties_set{};
  decltype(&::pw_properties_setf) properties_setf{};
  decltype(&::pw_properties_get) properties_get{};
  decltype(&::pw_properties_copy) properties_copy{};
  decltype(&::pw_properties_free) properties_free{};

  decltype(&::pw_stream_new) stream_new{};
  decltype(&::pw_stream_new_simple) stream_new_simple{};
  decltype(&::pw_stream_destroy) stream_destroy{};
  decltype(&::pw_stream_add_listener) stream_add_listener{};
  decltype(&::pw_stream_connect) stream_connect{};
  decltype(&::pw_stream_disconnect) stream_disconnect{};
  decltype(&::pw_stream_dequeue_buffer) stream_dequeue_buffer{};
  decltype(&::pw_stream_queue_buffer) stream_queue_buffer{};
  decltype(&::pw_stream_update_params) stream_update_params{};
  decltype(&::pw_stream_update_properties) stream_update_properties{};
  decltype(&::pw_stream_set_active) stream_set_active{};
#if PW_CHECK_VERSION(0, 3, 70)
  decltype(&::pw_stream_set_param) stream_set_param{};
#else
  int (*stream_set_param)(){}; // pipewire < 0.3.70: symbol absent
#endif
  decltype(&::pw_stream_get_state) stream_get_state{};
  decltype(&::pw_stream_get_name) stream_get_name{};
  decltype(&::pw_stream_get_properties) stream_get_properties{};
  decltype(&::pw_stream_get_node_id) stream_get_node_id{};
  decltype(&::pw_stream_get_time_n) stream_get_time_n{};
  decltype(&::pw_stream_state_as_string) stream_state_as_string{};
  decltype(&::pw_stream_trigger_process) stream_trigger_process{};
  decltype(&::pw_stream_flush) stream_flush{};

  decltype(&::pw_filter_new) filter_new{};
  decltype(&::pw_filter_new_simple) filter_new_simple{};
  decltype(&::pw_filter_destroy) filter_destroy{};
  decltype(&::pw_filter_add_listener) filter_add_listener{};
  decltype(&::pw_filter_connect) filter_connect{};
  decltype(&::pw_filter_disconnect) filter_disconnect{};
  decltype(&::pw_filter_get_state) filter_get_state{};
  decltype(&::pw_filter_get_node_id) filter_get_node_id{};
  decltype(&::pw_filter_get_properties) filter_get_properties{};
  decltype(&::pw_filter_update_properties) filter_update_properties{};
  decltype(&::pw_filter_update_params) filter_update_params{};
  decltype(&::pw_filter_add_port) filter_add_port{};
  decltype(&::pw_filter_remove_port) filter_remove_port{};
  decltype(&::pw_filter_dequeue_buffer) filter_dequeue_buffer{};
  decltype(&::pw_filter_queue_buffer) filter_queue_buffer{};
  decltype(&::pw_filter_get_dsp_buffer) filter_get_dsp_buffer{};

  decltype(&::pw_filter_set_active) filter_set_active{};
  decltype(&::pw_filter_flush) filter_flush{};
#if PW_CHECK_VERSION(0, 3, 66)
  decltype(&::pw_filter_trigger_process) filter_trigger_process{};
#else
  int (*filter_trigger_process)(){}; // pipewire < 0.3.66: symbol absent
#endif

  bool available{};
  bool core_available{};
  bool thread_available{};
  bool stream_available{};
  bool filter_available{};

  static const api& instance() noexcept
  {
    static const api self;
    return self;
  }

private:
  static void* try_open() noexcept
  {
    for (const char* name : {"libpipewire-0.3.so.0", "libpipewire-0.3.so"})
    {
      if (void* h = ::dlopen(name, RTLD_LAZY | RTLD_LOCAL | RTLD_NODELETE))
        return h;
    }
    return nullptr;
  }

  template <typename T>
  T sym(void* handle, const char* name) noexcept
  {
    return reinterpret_cast<T>(::dlsym(handle, name));
  }

  api() noexcept
  {
    void* h = try_open();
    if (!h)
      return;

    init = sym<decltype(init)>(h, "pw_init");
    deinit = sym<decltype(deinit)>(h, "pw_deinit");
    get_library_version = sym<decltype(get_library_version)>(h, "pw_get_library_version");

    main_loop_new = sym<decltype(main_loop_new)>(h, "pw_main_loop_new");
    main_loop_destroy = sym<decltype(main_loop_destroy)>(h, "pw_main_loop_destroy");
    main_loop_run = sym<decltype(main_loop_run)>(h, "pw_main_loop_run");
    main_loop_quit = sym<decltype(main_loop_quit)>(h, "pw_main_loop_quit");
    main_loop_get_loop = sym<decltype(main_loop_get_loop)>(h, "pw_main_loop_get_loop");

    context_new = sym<decltype(context_new)>(h, "pw_context_new");
    context_destroy = sym<decltype(context_destroy)>(h, "pw_context_destroy");
    context_connect = sym<decltype(context_connect)>(h, "pw_context_connect");
    context_connect_fd = sym<decltype(context_connect_fd)>(h, "pw_context_connect_fd");
    context_load_module = sym<decltype(context_load_module)>(h, "pw_context_load_module");
    core_disconnect = sym<decltype(core_disconnect)>(h, "pw_core_disconnect");

    proxy_add_listener = sym<decltype(proxy_add_listener)>(h, "pw_proxy_add_listener");
    proxy_destroy = sym<decltype(proxy_destroy)>(h, "pw_proxy_destroy");

    properties_new = sym<decltype(properties_new)>(h, "pw_properties_new");
    properties_new_dict = sym<decltype(properties_new_dict)>(h, "pw_properties_new_dict");
    properties_set = sym<decltype(properties_set)>(h, "pw_properties_set");
    properties_setf = sym<decltype(properties_setf)>(h, "pw_properties_setf");
    properties_get = sym<decltype(properties_get)>(h, "pw_properties_get");
    properties_copy = sym<decltype(properties_copy)>(h, "pw_properties_copy");
    properties_free = sym<decltype(properties_free)>(h, "pw_properties_free");

    core_available = init && deinit && main_loop_new && main_loop_destroy && main_loop_run
                     && main_loop_quit && main_loop_get_loop && context_new && context_destroy
                     && context_connect && core_disconnect && proxy_destroy && properties_new
                     && properties_free;
    available = core_available;

    thread_loop_new = sym<decltype(thread_loop_new)>(h, "pw_thread_loop_new");
    thread_loop_new_full = sym<decltype(thread_loop_new_full)>(h, "pw_thread_loop_new_full");
    thread_loop_destroy = sym<decltype(thread_loop_destroy)>(h, "pw_thread_loop_destroy");
    thread_loop_start = sym<decltype(thread_loop_start)>(h, "pw_thread_loop_start");
    thread_loop_stop = sym<decltype(thread_loop_stop)>(h, "pw_thread_loop_stop");
    thread_loop_get_loop = sym<decltype(thread_loop_get_loop)>(h, "pw_thread_loop_get_loop");
    thread_loop_lock = sym<decltype(thread_loop_lock)>(h, "pw_thread_loop_lock");
    thread_loop_unlock = sym<decltype(thread_loop_unlock)>(h, "pw_thread_loop_unlock");
    thread_loop_signal = sym<decltype(thread_loop_signal)>(h, "pw_thread_loop_signal");
    thread_loop_wait = sym<decltype(thread_loop_wait)>(h, "pw_thread_loop_wait");
    thread_loop_timed_wait = sym<decltype(thread_loop_timed_wait)>(h, "pw_thread_loop_timed_wait");
    thread_loop_in_thread = sym<decltype(thread_loop_in_thread)>(h, "pw_thread_loop_in_thread");
    thread_loop_add_listener
        = sym<decltype(thread_loop_add_listener)>(h, "pw_thread_loop_add_listener");
    thread_loop_accept = sym<decltype(thread_loop_accept)>(h, "pw_thread_loop_accept");

    thread_available = thread_loop_new && thread_loop_destroy && thread_loop_start
                       && thread_loop_stop && thread_loop_get_loop && thread_loop_lock
                       && thread_loop_unlock && thread_loop_signal && thread_loop_wait
                       && thread_loop_in_thread;

    stream_new = sym<decltype(stream_new)>(h, "pw_stream_new");
    stream_new_simple = sym<decltype(stream_new_simple)>(h, "pw_stream_new_simple");
    stream_destroy = sym<decltype(stream_destroy)>(h, "pw_stream_destroy");
    stream_add_listener = sym<decltype(stream_add_listener)>(h, "pw_stream_add_listener");
    stream_connect = sym<decltype(stream_connect)>(h, "pw_stream_connect");
    stream_disconnect = sym<decltype(stream_disconnect)>(h, "pw_stream_disconnect");
    stream_dequeue_buffer = sym<decltype(stream_dequeue_buffer)>(h, "pw_stream_dequeue_buffer");
    stream_queue_buffer = sym<decltype(stream_queue_buffer)>(h, "pw_stream_queue_buffer");
    stream_update_params = sym<decltype(stream_update_params)>(h, "pw_stream_update_params");
    stream_update_properties
        = sym<decltype(stream_update_properties)>(h, "pw_stream_update_properties");
    stream_set_active = sym<decltype(stream_set_active)>(h, "pw_stream_set_active");
#if PW_CHECK_VERSION(0, 3, 70)
    stream_set_param = sym<decltype(stream_set_param)>(h, "pw_stream_set_param");
#endif
    stream_get_state = sym<decltype(stream_get_state)>(h, "pw_stream_get_state");
    stream_get_name = sym<decltype(stream_get_name)>(h, "pw_stream_get_name");
    stream_get_properties = sym<decltype(stream_get_properties)>(h, "pw_stream_get_properties");
    stream_get_node_id = sym<decltype(stream_get_node_id)>(h, "pw_stream_get_node_id");
    stream_get_time_n = sym<decltype(stream_get_time_n)>(h, "pw_stream_get_time_n");
    stream_state_as_string = sym<decltype(stream_state_as_string)>(h, "pw_stream_state_as_string");
    stream_trigger_process = sym<decltype(stream_trigger_process)>(h, "pw_stream_trigger_process");
    stream_flush = sym<decltype(stream_flush)>(h, "pw_stream_flush");

    stream_available = stream_new && stream_destroy && stream_add_listener && stream_connect
                       && stream_disconnect && stream_dequeue_buffer && stream_queue_buffer
                       && stream_update_params;

    filter_new = sym<decltype(filter_new)>(h, "pw_filter_new");
    filter_new_simple = sym<decltype(filter_new_simple)>(h, "pw_filter_new_simple");
    filter_destroy = sym<decltype(filter_destroy)>(h, "pw_filter_destroy");
    filter_add_listener = sym<decltype(filter_add_listener)>(h, "pw_filter_add_listener");
    filter_connect = sym<decltype(filter_connect)>(h, "pw_filter_connect");
    filter_disconnect = sym<decltype(filter_disconnect)>(h, "pw_filter_disconnect");
    filter_get_state = sym<decltype(filter_get_state)>(h, "pw_filter_get_state");
    filter_get_node_id = sym<decltype(filter_get_node_id)>(h, "pw_filter_get_node_id");
    filter_get_properties = sym<decltype(filter_get_properties)>(h, "pw_filter_get_properties");
    filter_update_properties
        = sym<decltype(filter_update_properties)>(h, "pw_filter_update_properties");
    filter_update_params = sym<decltype(filter_update_params)>(h, "pw_filter_update_params");
    filter_add_port = sym<decltype(filter_add_port)>(h, "pw_filter_add_port");
    filter_remove_port = sym<decltype(filter_remove_port)>(h, "pw_filter_remove_port");
    filter_dequeue_buffer = sym<decltype(filter_dequeue_buffer)>(h, "pw_filter_dequeue_buffer");
    filter_queue_buffer = sym<decltype(filter_queue_buffer)>(h, "pw_filter_queue_buffer");
    filter_get_dsp_buffer = sym<decltype(filter_get_dsp_buffer)>(h, "pw_filter_get_dsp_buffer");
    filter_set_active = sym<decltype(filter_set_active)>(h, "pw_filter_set_active");
    filter_flush = sym<decltype(filter_flush)>(h, "pw_filter_flush");
#if PW_CHECK_VERSION(0, 3, 66)
    filter_trigger_process = sym<decltype(filter_trigger_process)>(h, "pw_filter_trigger_process");
#endif

    filter_available = filter_new_simple && filter_destroy && filter_connect && filter_disconnect
                       && filter_add_port && filter_get_node_id && filter_get_dsp_buffer;
  }

  api(const api&) = delete;
  api& operator=(const api&) = delete;
  api(api&&) = delete;
  api& operator=(api&&) = delete;
};

inline const api& load() noexcept
{
  return api::instance();
}

}
