// clang-format off
#include <libremidi/libremidi-c.h>
#include <libremidi/libremidi.hpp>
#include <libremidi/backends.hpp>
// clang-format on

#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <limits>

struct libremidi_midi_observer_handle
{
  libremidi::observer self;
};

struct libremidi_midi_in_handle
{
  libremidi::midi_in self;
};

struct libremidi_midi_out_handle
{
  libremidi::midi_out self;
};

NAMESPACE_LIBREMIDI
{

LIBREMIDI_STATIC void assign_error_callback(const auto& src, auto& dst)
{
  if (src.callback)
  {
    dst = [src](std::string_view errorText, const source_location& loc) {
      src.callback(src.context, errorText.data(), errorText.size(), &loc);
    };
  }
}
}

extern "C" {
const char* libremidi_get_version(void)
{
  return LIBREMIDI_VERSION;
}

void libremidi_midi1_available_apis(void* ctx, void (*cb)(void* ctx, libremidi_api))
{
  if (!cb)
    return;
  libremidi::midi1::for_all_backends([=](auto b) { cb(ctx, b.API); });
}

void libremidi_midi2_available_apis(void* ctx, void (*cb)(void* ctx, libremidi_api))
{
  if (!cb)
    return;
  libremidi::midi2::for_all_backends([=](auto b) { cb(ctx, b.API); });
}

const char* libremidi_api_identifier(libremidi_api api)
{
  return libremidi::get_api_name(api).data();
}

const char* libremidi_api_display_name(libremidi_api api)
{
  return libremidi::get_api_display_name(api).data();
}

libremidi_api libremidi_get_compiled_api_by_identifier(const char* name)
{
  libremidi_api ret = libremidi_api::UNSPECIFIED;
  libremidi::midi_any::for_all_backends([&](auto& b) {
    if (name == b.name)
      ret = b.API;
  });
  return ret;
}

int libremidi_midi_api_configuration_init(libremidi_api_configuration* conf)
{
  memset(conf, 0, sizeof(*conf));
  return 0;
}

int libremidi_midi_observer_configuration_init(libremidi_observer_configuration* conf)
{
  memset(conf, 0, sizeof(*conf));
  return 0;
}

int libremidi_midi_configuration_init(libremidi_midi_configuration* conf)
{
  memset(conf, 0, sizeof(*conf));
  return 0;
}

int libremidi_midi_in_port_clone(const libremidi_midi_in_port* port, libremidi_midi_in_port** dst)
{
  if (!port || !dst)
    return -EINVAL;

  auto copied = new libremidi::input_port{*reinterpret_cast<const libremidi::input_port*>(port)};
  *dst = reinterpret_cast<libremidi_midi_in_port*>(copied);
  return 0;
}

int libremidi_midi_in_port_free(libremidi_midi_in_port* port)
{
  delete reinterpret_cast<libremidi::input_port*>(port);
  return 0;
}

int libremidi_midi_in_port_name(const libremidi_midi_in_port* port, const char** name, size_t* len)
{
  if (!port || !name || !len)
    return -EINVAL;

  auto& p = *reinterpret_cast<const libremidi::input_port*>(port);
  *name = p.port_name.data();
  *len = p.port_name.size();
  return 0;
}

int libremidi_midi_in_port_handle(const libremidi_midi_in_port* port, uint64_t* handle)
{
  if (!port || !handle)
    return -EINVAL;

  auto& p = *reinterpret_cast<const libremidi::input_port*>(port);
  *handle = static_cast<uint64_t>(p.port);
  return 0;
}

int libremidi_midi_out_port_clone(
    const libremidi_midi_out_port* port, libremidi_midi_out_port** dst)
{
  if (!port || !dst)
    return -EINVAL;

  auto copied = new libremidi::output_port{*reinterpret_cast<const libremidi::output_port*>(port)};
  *dst = reinterpret_cast<libremidi_midi_out_port*>(copied);
  return 0;
}

int libremidi_midi_out_port_free(libremidi_midi_out_port* port)
{
  delete reinterpret_cast<libremidi::output_port*>(port);
  return 0;
}

int libremidi_midi_out_port_name(
    const libremidi_midi_out_port* port, const char** name, size_t* len)
{
  if (!port || !name || !len)
    return -EINVAL;

  auto& p = *reinterpret_cast<const libremidi::output_port*>(port);
  *name = p.port_name.data();
  *len = p.port_name.size();
  return 0;
}

int libremidi_midi_out_port_handle(const libremidi_midi_out_port* port, uint64_t* handle)
{
  if (!port || !handle)
    return -EINVAL;

  auto& p = *reinterpret_cast<const libremidi::output_port*>(port);
  *handle = static_cast<uint64_t>(p.port);
  return 0;
}

int libremidi_midi_observer_new(
    const libremidi_observer_configuration* c, libremidi_api_configuration* api,
    libremidi_midi_observer_handle** out)
{
  if (!out || !c)
    return -EINVAL;

  libremidi::observer_configuration conf;
  libremidi::assign_error_callback(c->on_error, conf.on_error);
  libremidi::assign_error_callback(c->on_warning, conf.on_warning);

  conf.track_hardware = c->track_hardware;
  conf.track_virtual = c->track_virtual;
  conf.track_any = c->track_any;
  conf.notify_in_constructor = c->notify_in_constructor;

  auto api_conf = libremidi::observer_configuration_for(static_cast<libremidi::API>(api->api));

  if (c->input_added.callback)
  {
    conf.input_added = [cb = c->input_added](const auto& port) {
      cb.callback(cb.context, reinterpret_cast<const libremidi_midi_in_port*>(&port));
    };
  }
  if (c->input_removed.callback)
  {
    conf.input_removed = [cb = c->input_removed](const auto& port) {
      cb.callback(cb.context, reinterpret_cast<const libremidi_midi_in_port*>(&port));
    };
  }
  if (c->output_added.callback)
  {
    conf.output_added = [cb = c->output_added](const auto& port) {
      cb.callback(cb.context, reinterpret_cast<const libremidi_midi_out_port*>(&port));
    };
  }
  if (c->output_removed.callback)
  {
    conf.output_removed = [cb = c->output_removed](const auto& port) {
      cb.callback(cb.context, reinterpret_cast<const libremidi_midi_out_port*>(&port));
    };
  }

  try
  {
    auto ptr = new libremidi_midi_observer_handle{
        libremidi::observer{std::move(conf), std::move(api_conf)}};
    *out = ptr;
    return 0;
  }
  catch (...)
  {
    return -EINVAL;
  }
}

int libremidi_midi_observer_enumerate_input_ports(
    libremidi_midi_observer_handle* ptr, void* context,
    void (*cb)(void* ctx, const libremidi_midi_in_port*))
{
  if (!ptr || !cb)
    return -EINVAL;

  for (const auto& port : ptr->self.get_input_ports())
  {
    cb(context, reinterpret_cast<const libremidi_midi_in_port*>(&port));
  }

  return 0;
}

int libremidi_midi_observer_enumerate_output_ports(
    libremidi_midi_observer_handle* ptr, void* context,
    void (*cb)(void* ctx, const libremidi_midi_out_port*))
{
  if (!ptr || !cb)
    return -EINVAL;

  for (const auto& port : ptr->self.get_output_ports())
  {
    cb(context, reinterpret_cast<const libremidi_midi_out_port*>(&port));
  }

  return 0;
}

int libremidi_midi_observer_free(libremidi_midi_observer_handle* ptr)
{
  delete ptr;
  return 0;
}

int libremidi_midi_in_new(
    const libremidi_midi_configuration* c, const libremidi_api_configuration* api,
    libremidi_midi_in_handle** out)
{
  if (!out || !c)
    return -EINVAL;
  if (!c->virtual_port && !c->in_port)
    return -EINVAL;

  *out = nullptr;

  auto api_conf = libremidi::midi_in_configuration_for(static_cast<libremidi::API>(api->api));

  // Create the MIDI object
  switch (c->version)
  {
    case libremidi_midi_configuration::MIDI1:
    case libremidi_midi_configuration::MIDI1_RAW: {
      libremidi::input_configuration conf;
      libremidi::assign_error_callback(c->on_error, conf.on_error);
      libremidi::assign_error_callback(c->on_warning, conf.on_warning);
      conf.ignore_sensing = c->ignore_sensing;
      conf.ignore_sysex = c->ignore_sysex;
      conf.ignore_timing = c->ignore_timing;
      conf.timestamps = c->timestamps;

      if (c->get_timestamp.callback)
      {
        conf.get_timestamp
            = [cb = c->get_timestamp](int64_t msg) { return cb.callback(cb.context, msg); };
      }

      if (c->version == libremidi_midi_configuration::MIDI1 && c->on_midi1_message.callback)
        conf.on_message = [cb = c->on_midi1_message](const libremidi::message& msg) {
          cb.callback(cb.context, msg.timestamp, msg.bytes.data(), msg.size());
        };
      else if (
          c->version == libremidi_midi_configuration::MIDI1_RAW && c->on_midi1_raw_data.callback)
      {
        conf.on_raw_data = [cb = c->on_midi1_raw_data](std::span<const uint8_t> msg, int64_t ts) {
          cb.callback(cb.context, ts, msg.data(), msg.size());
        };
      }
      else
      {
        return -EINVAL;
      }

      try
      {
        auto ptr = new libremidi_midi_in_handle{
            libremidi::midi_in{std::move(conf), std::move(api_conf)}};
        *out = ptr;
      }
      catch (...)
      {
        return -EINVAL;
      }
      break;
    }
    case libremidi_midi_configuration::MIDI2:
    case libremidi_midi_configuration::MIDI2_RAW: {
      libremidi::ump_input_configuration conf;
      libremidi::assign_error_callback(c->on_error, conf.on_error);
      libremidi::assign_error_callback(c->on_warning, conf.on_warning);
      conf.ignore_sensing = c->ignore_sensing;
      conf.ignore_sysex = c->ignore_sysex;
      conf.ignore_timing = c->ignore_timing;
      conf.timestamps = c->timestamps;

      if (c->get_timestamp.callback)
      {
        conf.get_timestamp
            = [cb = c->get_timestamp](int64_t msg) { return cb.callback(cb.context, msg); };
      }

      if (c->version == libremidi_midi_configuration::MIDI2 && c->on_midi2_message.callback)
        conf.on_message = [cb = c->on_midi2_message](const libremidi::ump& msg) {
          cb.callback(cb.context, msg.timestamp, msg.data, msg.size());
        };
      else if (
          c->version == libremidi_midi_configuration::MIDI2_RAW && c->on_midi2_raw_data.callback)
      {
        conf.on_raw_data = [cb = c->on_midi2_raw_data](std::span<const uint32_t> msg, int64_t ts) {
          cb.callback(cb.context, ts, msg.data(), msg.size());
        };
      }
      else
      {
        return -EINVAL;
      }

      try
      {
        auto ptr = new libremidi_midi_in_handle{
            libremidi::midi_in{std::move(conf), std::move(api_conf)}};
        *out = ptr;
      }
      catch (...)
      {
        return -EINVAL;
      }
      break;
    }
    default:
      return -EINVAL;
  }

  // Open the port
  auto ptr = *out;
  if (!ptr)
    return -EINVAL;

  if (c->virtual_port)
  {
    if (auto ret = ptr->self.open_virtual_port(c->port_name); ret != stdx::error{})
    {
      *out = nullptr;
      delete ptr;
      return -EIO;
    }
  }
  else
  {
    auto port = reinterpret_cast<const libremidi::input_port*>(c->in_port);
    if (auto ret = ptr->self.open_port(*port); ret != stdx::error{})
    {
      *out = nullptr;
      delete ptr;
      return -EIO;
    }
  }

  return 0;
}
int libremidi_midi_in_is_connected(const libremidi_midi_in_handle* in)
{
  if (!in)
    return -EINVAL;
  return in->self.is_port_connected() ? 1 : 0;
}

libremidi_timestamp libremidi_midi_in_absolute_timestamp(libremidi_midi_in_handle* in)
{
  if (!in)
    return -EINVAL;
  return in->self.absolute_timestamp();
}

int libremidi_midi_in_free(libremidi_midi_in_handle* ptr)
{
  delete ptr;
  return 0;
}

int libremidi_midi_out_new(
    const libremidi_midi_configuration* c, const libremidi_api_configuration* api,
    libremidi_midi_out_handle** out)
{
  if (!out || !c)
    return -EINVAL;
  if (!c->virtual_port && !c->out_port)
    return -EINVAL;

  *out = nullptr;

  auto api_conf = libremidi::midi_out_configuration_for(static_cast<libremidi::API>(api->api));

  // Create the MIDI object
  libremidi::output_configuration conf;
  libremidi::assign_error_callback(c->on_error, conf.on_error);
  libremidi::assign_error_callback(c->on_warning, conf.on_warning);
  conf.timestamps = c->timestamps;
  try
  {
    auto ptr
        = new libremidi_midi_out_handle{libremidi::midi_out{std::move(conf), std::move(api_conf)}};
    *out = ptr;
  }
  catch (...)
  {
    return -EINVAL;
  }

  // Open the port
  auto ptr = *out;
  if (!ptr)
    return -EINVAL;

  if (c->virtual_port)
  {
    if (auto ret = ptr->self.open_virtual_port(c->port_name); ret != stdx::error{})
    {
      *out = nullptr;
      delete ptr;
      return -EIO;
    }
  }
  else
  {
    auto port = reinterpret_cast<const libremidi::output_port*>(c->in_port);
    if (auto ret = ptr->self.open_port(*port); ret != stdx::error{})
    {
      *out = nullptr;
      delete ptr;
      return -EIO;
    }
  }
  return 0;
}

int libremidi_midi_out_is_connected(const libremidi_midi_out_handle* out)
{
  if (!out)
    return -EINVAL;
  return out->self.is_port_connected() ? 1 : 0;
}

int libremidi_midi_out_send_message(
    libremidi_midi_out_handle* out, const libremidi_midi1_symbol* msg, size_t sz)
{
  if (!out || !msg || sz > std::numeric_limits<int32_t>::max())
    return -EINVAL;

  auto res = out->self.send_message(msg, sz);
  return res != stdx::error{} ? -EIO : 0;
}

int libremidi_midi_out_send_ump(
    libremidi_midi_out_handle* out, const libremidi_midi2_symbol* msg, size_t sz)
{
  if (!out || !msg || sz > std::numeric_limits<int32_t>::max())
    return -EINVAL;

  auto res = out->self.send_ump(msg, sz);
  return res != stdx::error{} ? -EIO : 0;
}

int libremidi_midi_out_schedule_message(
    libremidi_midi_out_handle* out, int64_t ts, const libremidi_midi1_symbol* msg, size_t sz)
{
  if (!out || !msg || sz > std::numeric_limits<int32_t>::max())
    return -EINVAL;

  auto res = out->self.schedule_message(ts, msg, sz);
  return res != stdx::error{} ? -EIO : 0;
}

int libremidi_midi_out_schedule_ump(
    libremidi_midi_out_handle* out, int64_t ts, const libremidi_midi2_symbol* msg, size_t sz)
{
  if (!out || !msg || sz > std::numeric_limits<int32_t>::max())
    return -EINVAL;

  auto res = out->self.schedule_ump(ts, msg, sz);
  return res != stdx::error{} ? -EIO : 0;
}

int libremidi_midi_out_free(libremidi_midi_out_handle* ptr)
{
  delete ptr;
  return 0;
}
}
