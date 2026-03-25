#include <libremidi/libremidi.hpp>

#include <boost/variant2.hpp>
#include <nanobind/nanobind.h>
#include <nanobind/stl/array.h>
#include <nanobind/stl/function.h>
#include <nanobind/stl/pair.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/string_view.h>
#include <nanobind/stl/tuple.h>
#include <nanobind/stl/vector.h>

#include <readerwriterqueue.h>

namespace libremidi {
namespace poll_queue {
struct error_message {
  std::string msg;
};
struct warning_message {
  std::string msg;
};
struct input_added_message {
  input_port port;
};
struct input_removed_message {
  input_port port;
};
struct output_added_message {
  output_port port;
};
struct output_removed_message {
  output_port port;
};
struct midi1_raw_message {
  std::vector<uint8_t> data;
  timestamp t;
};
struct midi2_raw_message {
  std::vector<uint32_t> data;
  timestamp t;
};

using observer_msg = boost::variant2::variant<error_message, warning_message, input_added_message, input_removed_message, output_added_message, output_removed_message>;
using midi_in_msg = boost::variant2::variant<error_message, warning_message, libremidi::message, midi1_raw_message, libremidi::ump, midi2_raw_message>;
using midi_out_msg = boost::variant2::variant<error_message, warning_message>;
} // namespace poll_queue

struct observer_poll_wrapper {
  moodycamel::ReaderWriterQueue<poll_queue::observer_msg> queue{};
  observer_configuration conf;
  observer impl;
  explicit observer_poll_wrapper(observer_configuration conf = {}) noexcept : conf{conf}, impl{this->process(std::move(conf))} {}

  explicit observer_poll_wrapper(observer_configuration conf, libremidi::observer_api_configuration api_conf) : conf{conf}, impl{process(std::move(conf)), std::move(api_conf)} {}

  observer_configuration process(observer_configuration &&obs) {
    if (obs.on_error)
      obs.on_error = [this](std::string_view errorText, const source_location &) { queue.enqueue(poll_queue::error_message{std::string{errorText}}); };
    if (obs.on_warning)
      obs.on_warning = [this](std::string_view errorText, const source_location &) { queue.enqueue(poll_queue::warning_message{std::string{errorText}}); };

    if (obs.input_added)
      obs.input_added = [this](const input_port &val) { queue.enqueue(poll_queue::input_added_message{val}); };
    if (obs.input_removed)
      obs.input_removed = [this](const input_port &val) { queue.enqueue(poll_queue::input_removed_message{val}); };
    if (obs.output_added)
      obs.output_added = [this](const output_port &val) { queue.enqueue(poll_queue::output_added_message{val}); };
    if (obs.output_removed)
      obs.output_removed = [this](const output_port &val) { queue.enqueue(poll_queue::output_removed_message{val}); };

    return obs;
  }

  void poll() {
    poll_queue::observer_msg m;
    while (queue.try_dequeue(m))
      boost::variant2::visit(*this, m);
  }

  void operator()(const poll_queue::error_message &msg) const noexcept { conf.on_error(msg.msg, {}); }
  void operator()(const poll_queue::warning_message &msg) const noexcept { conf.on_warning(msg.msg, {}); }
  void operator()(const poll_queue::input_added_message &msg) const noexcept { conf.input_added(msg.port); }
  void operator()(const poll_queue::input_removed_message &msg) const noexcept { conf.input_removed(msg.port); }
  void operator()(const poll_queue::output_added_message &msg) const noexcept { conf.output_added(msg.port); }
  void operator()(const poll_queue::output_removed_message &msg) const noexcept { conf.output_removed(msg.port); }
};


struct input_configuration_wrapper: libremidi::input_configuration
{
  std::function<void(std::vector<uint8_t>, libremidi::timestamp)> on_raw_data_vector;
};
struct ump_input_configuration_wrapper : libremidi::ump_input_configuration
{
  std::function<void(std::vector<uint32_t>, libremidi::timestamp)> on_raw_data_vector;
};

struct midi_in_poll_wrapper {
  moodycamel::ReaderWriterQueue<poll_queue::midi_in_msg> queue{};

  input_configuration_wrapper python_midi1_callbacks;
  ump_input_configuration_wrapper python_ump_callbacks;
  midi_in impl;

  explicit midi_in_poll_wrapper(const input_configuration_wrapper &conf) noexcept : python_midi1_callbacks{conf}, impl{this->process(std::move(conf))} {}
  explicit midi_in_poll_wrapper(input_configuration_wrapper conf, input_api_configuration api_conf) : python_midi1_callbacks{conf}, impl{this->process(std::move(conf)), std::move(api_conf)} {}
  explicit midi_in_poll_wrapper(ump_input_configuration_wrapper conf) noexcept : python_ump_callbacks{conf}, impl{this->process(std::move(conf))} {}
  explicit midi_in_poll_wrapper(ump_input_configuration_wrapper conf, input_api_configuration api_conf) : python_ump_callbacks{conf}, impl{this->process(std::move(conf)), std::move(api_conf)} {}

  input_configuration process(input_configuration_wrapper obs) {
    python_midi1_callbacks = obs;

    if (obs.on_error)
      obs.on_error = [this](std::string_view errorText, const source_location &) { queue.enqueue(poll_queue::error_message{std::string{errorText}}); };
    if (obs.on_warning)
      obs.on_warning = [this](std::string_view errorText, const source_location &) { queue.enqueue(poll_queue::warning_message{std::string{errorText}}); };

    if (obs.on_message)
      obs.on_message = [this](libremidi::message &&msg) { queue.enqueue(std::move(msg)); };
    if (obs.on_raw_data_vector)
      obs.on_raw_data = [this](std::span<const uint8_t> msg, timestamp t) { queue.enqueue(poll_queue::midi1_raw_message{{msg.begin(), msg.end()}, t}); };
    return obs;
  }

  ump_input_configuration process(ump_input_configuration_wrapper obs) {
    python_ump_callbacks = obs;

    if (obs.on_error)
      obs.on_error = [this](std::string_view errorText, const source_location &) { queue.enqueue(poll_queue::error_message{std::string{errorText}}); };
    if (obs.on_warning)
      obs.on_warning = [this](std::string_view errorText, const source_location &) { queue.enqueue(poll_queue::warning_message{std::string{errorText}}); };

    if (obs.on_message)
      obs.on_message = [this](libremidi::ump &&msg) { queue.enqueue(std::move(msg)); };
    if (obs.on_raw_data_vector)
      obs.on_raw_data = [this](std::span<const uint32_t> msg, timestamp t) { queue.enqueue(poll_queue::midi2_raw_message{{msg.begin(), msg.end()}, t}); };
    return obs;
  }

  void poll() {
    poll_queue::midi_in_msg m;
    while (queue.try_dequeue(m))
      boost::variant2::visit(*this, std::move(m));
  }

  void operator()(const poll_queue::error_message &msg) const noexcept {
    if (python_midi1_callbacks.on_error)
      python_midi1_callbacks.on_error(msg.msg, {});
    else if (python_ump_callbacks.on_error)
      python_ump_callbacks.on_error(msg.msg, {});
  }
  void operator()(const poll_queue::warning_message &msg) const noexcept {
    if (python_midi1_callbacks.on_warning)
      python_midi1_callbacks.on_warning(msg.msg, {});
    else if (python_ump_callbacks.on_warning)
      python_ump_callbacks.on_warning(msg.msg, {});
  }
  void operator()(libremidi::message &&msg) const noexcept {
    if(python_midi1_callbacks.on_message)
      python_midi1_callbacks.on_message(std::move(msg));
  }
  void operator()(const poll_queue::midi1_raw_message &msg) const noexcept {
    if(python_midi1_callbacks.on_raw_data_vector)
    {
      python_midi1_callbacks.on_raw_data_vector(msg.data, msg.t);
    }
  }
  void operator()(libremidi::ump &&msg) const noexcept {
    if(python_ump_callbacks.on_message)
      python_ump_callbacks.on_message(std::move(msg));
  }
  void operator()(const poll_queue::midi2_raw_message &msg) const noexcept {
    if(python_ump_callbacks.on_raw_data_vector)
      python_ump_callbacks.on_raw_data_vector(msg.data, msg.t);
  }
};

struct midi_out_poll_wrapper {
  moodycamel::ReaderWriterQueue<poll_queue::midi_out_msg> queue{};
  output_configuration python_midi1_callbacks;
  midi_out impl;
  explicit midi_out_poll_wrapper() noexcept : impl{} {}

  explicit midi_out_poll_wrapper(const output_configuration &conf) noexcept : python_midi1_callbacks{conf}, impl{this->process(std::move(conf))} {}
  explicit midi_out_poll_wrapper(output_configuration conf, output_api_configuration api_conf) : python_midi1_callbacks{conf}, impl{this->process(std::move(conf)), std::move(api_conf)} {}

  output_configuration process(output_configuration obs) {
    python_midi1_callbacks = obs;

    if (obs.on_error)
      obs.on_error = [this](std::string_view errorText, const source_location &) { queue.enqueue(poll_queue::error_message{std::string{errorText}}); };
    if (obs.on_warning)
      obs.on_warning = [this](std::string_view errorText, const source_location &) { queue.enqueue(poll_queue::warning_message{std::string{errorText}}); };
    return obs;
  }

  void poll() {
    poll_queue::midi_out_msg m;
    while (queue.try_dequeue(m))
      boost::variant2::visit(*this, std::move(m));
  }

  void operator()(const poll_queue::error_message &msg) const noexcept { python_midi1_callbacks.on_error(msg.msg, {}); }
  void operator()(const poll_queue::warning_message &msg) const noexcept { python_midi1_callbacks.on_warning(msg.msg, {}); }
};
} // namespace libremidi

NB_MODULE(pylibremidi, m) {
#if defined(LIBREMIDI_WINMIDI) || defined(LIBREMIDI_WINUWP)
  winrt::init_apartment();
#endif

  namespace nb = nanobind;
  nb::class_<stdx::error>(m, "Error")
      .def("__bool__", [](stdx::error e) { return e != stdx::error{}; })
      .def("__str__", [](stdx::error e) { return e.message().data(); })
      .def("__repr__", [](stdx::error e) { return e.message().data(); });
  nb::enum_<libremidi::API>(m, "API")
      .value("UNSPECIFIED", libremidi::API::UNSPECIFIED)

      .value("COREMIDI", libremidi::API::COREMIDI)
      .value("ALSA_SEQ", libremidi::API::ALSA_SEQ)
      .value("ALSA_RAW", libremidi::API::ALSA_RAW)
      .value("JACK_MIDI", libremidi::API::JACK_MIDI)
      .value("WINDOWS_MM", libremidi::API::WINDOWS_MM)
      .value("WINDOWS_UWP", libremidi::API::WINDOWS_UWP)
      .value("WEBMIDI", libremidi::API::WEBMIDI)
      .value("PIPEWIRE", libremidi::API::PIPEWIRE)
      .value("KEYBOARD", libremidi::API::KEYBOARD)
      .value("NETWORK", libremidi::API::NETWORK)

      .value("ALSA_RAW_UMP", libremidi::API::ALSA_RAW_UMP)
      .value("ALSA_SEQ_UMP", libremidi::API::ALSA_SEQ_UMP)
      .value("COREMIDI_UMP", libremidi::API::COREMIDI_UMP)
      .value("WINDOWS_MIDI_SERVICES", libremidi::API::WINDOWS_MIDI_SERVICES)
      .value("KEYBOARD_UMP", libremidi::API::KEYBOARD_UMP)
      .value("NETWORK_UMP", libremidi::API::NETWORK_UMP)
      .value("JACK_UMP", libremidi::API::JACK_UMP)
      .value("PIPEWIRE_UMP", libremidi::API::PIPEWIRE_UMP)

      .value("DUMMY", libremidi::API::DUMMY)
      .export_values();

  m.def("available_apis", libremidi::available_apis);
  m.def("available_ump_apis", libremidi::available_ump_apis);
  m.def("get_version", libremidi::get_version);
  m.def("get_api_name", libremidi::get_api_name);
  m.def("get_api_display_name", libremidi::get_api_display_name);
  m.def("get_compiled_api_by_name", libremidi::get_compiled_api_by_name);
  m.def("midi1_default_api", libremidi::midi1::default_api);
  m.def("midi2_default_api", libremidi::midi2::default_api);

  nb::class_<libremidi::message>(m, "Message")
      .def(nb::init<>())
      .def_rw("bytes", &libremidi::message::bytes)
      .def_rw("timestamp", &libremidi::message::timestamp)
      .def("__len__", [](const libremidi::message &m, int idx) { return m.size(); })
      .def("__getitem__", [](const libremidi::message &m, int idx) { return m[idx]; })
      .def("__setitem__", [](libremidi::message &m, int idx, uint8_t res) { return m[idx] = res; })
      .def("__repr__", [](const libremidi::message &m) {
        std::stringstream str;
        str << m.timestamp << ": ";
        str << std::hex;
        str << "[ ";
        for (int val : m)
          str << val << ' ';
        str << ']';
        return str.str();
      });
  nb::class_<libremidi::ump>(m, "Ump")
      .def(nb::init<>())
      .def_prop_rw(
          "data", [](const libremidi::ump &obj) { return std::array<uint32_t, 4>{obj.data[0], obj.data[1], obj.data[2], obj.data[3]}; },
          [](libremidi::ump &obj, std::array<uint32_t, 4> v) { std::copy_n(v.begin(), 4, obj.data); })
      .def_rw("timestamp", &libremidi::ump::timestamp)
      .def("__len__", [](const libremidi::ump &m, int idx) { return m.size(); })
      .def("__getitem__", [](const libremidi::ump &m, int idx) { return m[idx]; })
      .def("__setitem__", [](libremidi::ump &m, int idx, uint32_t res) { return m[idx] = res; })
      .def("__repr__", [](const libremidi::ump &m) {
        std::stringstream str;
        str << m.timestamp << ": ";
        str << std::hex;
        str << "[ ";
        for (uint32_t val : m)
          str << val << ' ';
        str << ']';
        return str.str();
      });

  nb::class_<libremidi::port_information>(m, "PortInformation")
      .def(nb::init<>())
      .def_rw("client", &libremidi::port_information::client)
      .def_rw("port", &libremidi::port_information::port)
      .def_rw("manufacturer", &libremidi::port_information::manufacturer)
      .def_rw("device_name", &libremidi::port_information::device_name)
      .def_rw("port_name", &libremidi::port_information::port_name)
      .def_rw("display_name", &libremidi::port_information::display_name);
  nb::class_<libremidi::input_port, libremidi::port_information>(m, "InputPort").def(nb::init<>());
  nb::class_<libremidi::output_port, libremidi::port_information>(m, "OutputPort").def(nb::init<>());

  nb::class_<libremidi::observer_configuration>(m, "ObserverConfiguration")
      .def(nb::init<>())
      .def_rw("on_error", &libremidi::observer_configuration::on_error)
      .def_rw("on_warning", &libremidi::observer_configuration::on_warning)
      .def_rw("input_added", &libremidi::observer_configuration::input_added)
      .def_rw("input_removed", &libremidi::observer_configuration::input_removed)
      .def_rw("output_added", &libremidi::observer_configuration::output_added)
      .def_rw("output_removed", &libremidi::observer_configuration::output_removed)
      .def_prop_rw(
          "track_hardware", [](const libremidi::observer_configuration &obj) { return obj.track_hardware; }, [](libremidi::observer_configuration &obj, bool v) { obj.track_hardware = v; })
      .def_prop_rw(
          "track_virtual", [](const libremidi::observer_configuration &obj) { return obj.track_virtual; }, [](libremidi::observer_configuration &obj, bool v) { obj.track_virtual = v; })
      .def_prop_rw(
          "track_any", [](const libremidi::observer_configuration &obj) { return obj.track_any; }, [](libremidi::observer_configuration &obj, bool v) { obj.track_any = v; })
      .def_prop_rw(
          "notify_in_constructor", [](const libremidi::observer_configuration &obj) { return obj.notify_in_constructor; },
          [](libremidi::observer_configuration &obj, bool v) { obj.notify_in_constructor = v; });

  nb::class_<libremidi::input_configuration_wrapper>(m, "InputConfiguration")
      .def(nb::init<>())
      .def_rw("on_message", &libremidi::input_configuration_wrapper::on_message)
      .def_rw("on_raw_data", &libremidi::input_configuration_wrapper::on_raw_data_vector)
      .def_rw("get_timestamp", &libremidi::input_configuration_wrapper::get_timestamp)
      .def_rw("on_error", &libremidi::input_configuration_wrapper::on_error)
      .def_rw("on_warning", &libremidi::input_configuration_wrapper::on_warning)
      .def_prop_rw(
          "ignore_sysex", [](const libremidi::input_configuration_wrapper &obj) { return obj.ignore_sysex; }, [](libremidi::input_configuration_wrapper &obj, bool v) { obj.ignore_sysex = v; })
      .def_prop_rw(
          "ignore_timing", [](const libremidi::input_configuration_wrapper &obj) { return obj.ignore_timing; }, [](libremidi::input_configuration_wrapper &obj, bool v) { obj.ignore_timing = v; })
      .def_prop_rw(
          "ignore_sensing", [](const libremidi::input_configuration_wrapper &obj) { return obj.ignore_sensing; }, [](libremidi::input_configuration_wrapper &obj, bool v) { obj.ignore_sensing = v; })
      .def_prop_rw(
          "timestamps", [](const libremidi::input_configuration_wrapper &obj) { return obj.timestamps; }, [](libremidi::input_configuration_wrapper &obj, libremidi::timestamp_mode v) { obj.timestamps = v; });

  nb::class_<libremidi::ump_input_configuration_wrapper>(m, "UmpInputConfiguration")
      .def(nb::init<>())
      .def_rw("on_message", &libremidi::ump_input_configuration_wrapper::on_message)
      .def_rw("on_raw_data", &libremidi::ump_input_configuration_wrapper::on_raw_data_vector)
      .def_rw("get_timestamp", &libremidi::ump_input_configuration_wrapper::get_timestamp)
      .def_rw("on_error", &libremidi::ump_input_configuration_wrapper::on_error)
      .def_rw("on_warning", &libremidi::ump_input_configuration_wrapper::on_warning)
      .def_prop_rw(
          "ignore_sysex", [](const libremidi::ump_input_configuration_wrapper &obj) { return obj.ignore_sysex; }, [](libremidi::ump_input_configuration_wrapper &obj, bool v) { obj.ignore_sysex = v; })
      .def_prop_rw(
          "ignore_timing", [](const libremidi::ump_input_configuration_wrapper &obj) { return obj.ignore_timing; }, [](libremidi::ump_input_configuration_wrapper &obj, bool v) { obj.ignore_timing = v; })
      .def_prop_rw(
          "ignore_sensing", [](const libremidi::ump_input_configuration_wrapper &obj) { return obj.ignore_sensing; }, [](libremidi::ump_input_configuration_wrapper &obj, bool v) { obj.ignore_sensing = v; })
      .def_prop_rw(
          "timestamps", [](const libremidi::ump_input_configuration_wrapper &obj) { return obj.timestamps; }, [](libremidi::ump_input_configuration_wrapper &obj, libremidi::timestamp_mode v) { obj.timestamps = v; });

  nb::class_<libremidi::output_configuration>(m, "OutputConfiguration")
      .def(nb::init<>())
      .def_rw("on_error", &libremidi::output_configuration::on_error)
      .def_rw("on_warning", &libremidi::output_configuration::on_warning)
      .def_prop_rw(
          "timestamps", [](const libremidi::output_configuration &obj) { return obj.timestamps; }, [](libremidi::output_configuration &obj, libremidi::timestamp_mode v) { obj.timestamps = v; });

  nb::class_<libremidi::observer_api_configuration>(m, "ObserverApiConfiguration");
  nb::class_<libremidi::input_api_configuration>(m, "InputApiConfiguration");
  nb::class_<libremidi::output_api_configuration>(m, "OutputApiConfiguration");

  nb::class_<libremidi::unspecified_configuration>(m, "UnspecifiedConfiguration");
  nb::class_<libremidi::dummy_configuration>(m, "DummyConfiguration");

  nb::class_<libremidi::alsa_raw_input_configuration>(m, "AlsaRawInputConfiguration").def(nb::init<>()).def_rw("poll_period", &libremidi::alsa_raw_input_configuration::poll_period);
  nb::class_<libremidi::alsa_raw_ump::input_configuration>(m, "AlsaRawUmpInputConfiguration").def(nb::init<>()).def_rw("poll_period", &libremidi::alsa_raw_ump::input_configuration::poll_period);
  nb::class_<libremidi::alsa_seq::input_configuration>(m, "AlsaSeqInputConfiguration").def(nb::init<>()).def_rw("client_name", &libremidi::alsa_seq::input_configuration::client_name);
  nb::class_<libremidi::alsa_seq_ump::input_configuration>(m, "AlsaSeqUmpInputConfiguration").def(nb::init<>()).def_rw("client_name", &libremidi::alsa_seq_ump::input_configuration::client_name);
  nb::class_<libremidi::coremidi_input_configuration>(m, "CoremidiInputConfiguration").def(nb::init<>()).def_rw("client_name", &libremidi::coremidi_input_configuration::client_name);
  nb::class_<libremidi::coremidi_ump::input_configuration>(m, "CoremidiUmpInputConfiguration").def(nb::init<>()).def_rw("client_name", &libremidi::coremidi_ump::input_configuration::client_name);
  nb::class_<libremidi::emscripten_input_configuration>(m, "EmscriptenInputConfiguration").def(nb::init<>());
  nb::class_<libremidi::jack_input_configuration>(m, "JackInputConfiguration").def(nb::init<>()).def_rw("client_name", &libremidi::jack_input_configuration::client_name);
  nb::class_<libremidi::jack_ump::input_configuration>(m, "JackUmpInputConfiguration").def(nb::init<>()).def_rw("client_name", &libremidi::jack_ump::input_configuration::client_name);
  nb::class_<libremidi::kbd_input_configuration>(m, "KeyboardInputConfiguration").def(nb::init<>());
  nb::class_<libremidi::net::dgram_input_configuration>(m, "DatagramInputConfiguration").def(nb::init<>()).def_rw("client_name", &libremidi::net::dgram_input_configuration::client_name);
  nb::class_<libremidi::net_ump::dgram_input_configuration>(m, "DatagramUmpInputConfiguration").def(nb::init<>()).def_rw("client_name", &libremidi::net_ump::dgram_input_configuration::client_name);
  nb::class_<libremidi::pipewire_input_configuration>(m, "PipewireInputConfiguration").def(nb::init<>()).def_rw("client_name", &libremidi::pipewire_input_configuration::client_name);
  nb::class_<libremidi::pipewire_ump::input_configuration>(m, "PipewireUmpInputConfiguration").def(nb::init<>()).def_rw("client_name", &libremidi::pipewire_ump::input_configuration::client_name);
  nb::class_<libremidi::winmidi::input_configuration>(m, "WinmidiInputConfiguration").def(nb::init<>());
  nb::class_<libremidi::winmm_input_configuration>(m, "WinmmInputConfiguration").def(nb::init<>());
  nb::class_<libremidi::winuwp_input_configuration>(m, "WinuwpInputConfiguration").def(nb::init<>());

  nb::class_<libremidi::alsa_raw_output_configuration>(m, "AlsaRawOutputConfiguration").def(nb::init<>());
  nb::class_<libremidi::alsa_raw_ump::output_configuration>(m, "AlsaRawUmpOutputConfiguration").def(nb::init<>());
  nb::class_<libremidi::alsa_seq::output_configuration>(m, "AlsaSeqOutputConfiguration").def(nb::init<>()).def_rw("client_name", &libremidi::alsa_seq::output_configuration::client_name);
  nb::class_<libremidi::alsa_seq_ump::output_configuration>(m, "AlsaSeqUmpOutputConfiguration").def(nb::init<>()).def_rw("client_name", &libremidi::alsa_seq_ump::output_configuration::client_name);
  nb::class_<libremidi::coremidi_output_configuration>(m, "CoremidiOutputConfiguration").def(nb::init<>()).def_rw("client_name", &libremidi::coremidi_output_configuration::client_name);
  nb::class_<libremidi::coremidi_ump::output_configuration>(m, "CoremidiUmpOutputConfiguration").def(nb::init<>()).def_rw("client_name", &libremidi::coremidi_ump::output_configuration::client_name);
  nb::class_<libremidi::emscripten_output_configuration>(m, "EmscriptenOutputConfiguration").def(nb::init<>());
  nb::class_<libremidi::jack_output_configuration>(m, "JackOutputConfiguration").def(nb::init<>()).def_rw("client_name", &libremidi::jack_output_configuration::client_name);
  nb::class_<libremidi::jack_ump::output_configuration>(m, "JackUmpOutputConfiguration").def(nb::init<>()).def_rw("client_name", &libremidi::jack_ump::output_configuration::client_name);
  nb::class_<libremidi::net::dgram_output_configuration>(m, "DatagramOutputConfiguration").def(nb::init<>()).def_rw("client_name", &libremidi::net::dgram_output_configuration::client_name);
  nb::class_<libremidi::net_ump::dgram_output_configuration>(m, "DatagramUmpOutputConfiguration").def(nb::init<>()).def_rw("client_name", &libremidi::net_ump::dgram_output_configuration::client_name);
  nb::class_<libremidi::pipewire_output_configuration>(m, "PipewireOutputConfiguration").def(nb::init<>()).def_rw("client_name", &libremidi::pipewire_output_configuration::client_name);
  nb::class_<libremidi::pipewire_ump::output_configuration>(m, "PipewireUmpOutputConfiguration").def(nb::init<>()).def_rw("client_name", &libremidi::pipewire_ump::output_configuration::client_name);
  nb::class_<libremidi::winmidi::output_configuration>(m, "WinmidiOutputConfiguration").def(nb::init<>());
  nb::class_<libremidi::winmm_output_configuration>(m, "WinmmOutputConfiguration").def(nb::init<>());
  nb::class_<libremidi::winuwp_output_configuration>(m, "WinuwpOutputConfiguration").def(nb::init<>());

  nb::class_<libremidi::alsa_raw_observer_configuration>(m, "AlsaRawObserverConfiguration").def(nb::init<>());
  nb::class_<libremidi::alsa_raw_ump::observer_configuration>(m, "AlsaRawUmpObserverConfiguration").def(nb::init<>());
  nb::class_<libremidi::alsa_seq::observer_configuration>(m, "AlsaSeqObserverConfiguration").def(nb::init<>()).def_rw("client_name", &libremidi::alsa_seq::observer_configuration::client_name);
  nb::class_<libremidi::alsa_seq_ump::observer_configuration>(m, "AlsaSeqUmpObserverConfiguration")
      .def(nb::init<>())
      .def_rw("client_name", &libremidi::alsa_seq_ump::observer_configuration::client_name);
  nb::class_<libremidi::coremidi_observer_configuration>(m, "CoremidiObserverConfiguration").def(nb::init<>()).def_rw("client_name", &libremidi::coremidi_observer_configuration::client_name);
  nb::class_<libremidi::coremidi_ump::observer_configuration>(m, "CoremidiUmpObserverConfiguration")
      .def(nb::init<>())
      .def_rw("client_name", &libremidi::coremidi_ump::observer_configuration::client_name);
  nb::class_<libremidi::emscripten_observer_configuration>(m, "EmscriptenObserverConfiguration").def(nb::init<>());
  nb::class_<libremidi::jack_observer_configuration>(m, "JackObserverConfiguration").def(nb::init<>()).def_rw("client_name", &libremidi::jack_observer_configuration::client_name);
  nb::class_<libremidi::jack_ump::observer_configuration>(m, "JackUmpObserverConfiguration").def(nb::init<>()).def_rw("client_name", &libremidi::jack_ump::observer_configuration::client_name);
  nb::class_<libremidi::net::dgram_observer_configuration>(m, "DatagramObserverConfiguration").def(nb::init<>()).def_rw("client_name", &libremidi::net::dgram_observer_configuration::client_name);
  nb::class_<libremidi::net_ump::dgram_observer_configuration>(m, "DatagramUmpObserverConfiguration")
      .def(nb::init<>())
      .def_rw("client_name", &libremidi::net_ump::dgram_observer_configuration::client_name);
  nb::class_<libremidi::pipewire_observer_configuration>(m, "PipewireObserverConfiguration").def(nb::init<>()).def_rw("client_name", &libremidi::pipewire_observer_configuration::client_name);
  nb::class_<libremidi::pipewire_ump::observer_configuration>(m, "PipewireObserverConfiguration")
      .def(nb::init<>())
      .def_rw("client_name", &libremidi::pipewire_ump::observer_configuration::client_name);
  nb::class_<libremidi::winmidi::observer_configuration>(m, "WinmidiObserverConfiguration").def(nb::init<>());
  nb::class_<libremidi::winmm_observer_configuration>(m, "WinmmObserverConfiguration").def(nb::init<>());
  nb::class_<libremidi::winuwp_observer_configuration>(m, "WinuwpObserverConfiguration").def(nb::init<>());

  nb::class_<libremidi::observer_poll_wrapper>(m, "Observer")
      .def(nb::init<>())
      .def(nb::init<libremidi::observer_configuration>())
      .def(nb::init<libremidi::observer_configuration, libremidi::API>())
      .def("get_current_api", [](libremidi::observer_poll_wrapper &self) { return self.impl.get_current_api(); })
      .def("get_input_ports", [](libremidi::observer_poll_wrapper &self) { return self.impl.get_input_ports(); })
      .def("get_output_ports", [](libremidi::observer_poll_wrapper &self) { return self.impl.get_output_ports(); })
      .def("poll", [](libremidi::observer_poll_wrapper &self) { return self.poll(); });

  nb::class_<libremidi::midi_in_poll_wrapper>(m, "MidiIn")
      .def(nb::init<libremidi::input_configuration_wrapper>())
      .def(nb::init<libremidi::input_configuration_wrapper, libremidi::API>())
      .def(nb::init<libremidi::ump_input_configuration_wrapper>())
      .def(nb::init<libremidi::ump_input_configuration_wrapper, libremidi::API>())
      .def("get_current_api", [](libremidi::midi_in_poll_wrapper &self) { return self.impl.get_current_api(); })
      .def("open_port", [](libremidi::midi_in_poll_wrapper &self, const libremidi::input_port &p) { return self.impl.open_port(p); })
      .def("open_port", [](libremidi::midi_in_poll_wrapper &self, const libremidi::input_port &p, std::string_view name) { return self.impl.open_port(p, name); })
      .def("open_virtual_port", [](libremidi::midi_in_poll_wrapper &self) { return self.impl.open_virtual_port(); })
      .def("open_virtual_port", [](libremidi::midi_in_poll_wrapper &self, std::string_view name) { return self.impl.open_virtual_port(name); })
      .def("set_port_name", [](libremidi::midi_in_poll_wrapper &self, std::string_view name) { return self.impl.set_port_name(name); })
      .def("close_port", [](libremidi::midi_in_poll_wrapper &self) { return self.impl.close_port(); })
      .def("is_port_open", [](libremidi::midi_in_poll_wrapper &self) { return self.impl.is_port_open(); })
      .def("is_port_connected", [](libremidi::midi_in_poll_wrapper &self) { return self.impl.is_port_connected(); })
      .def("absolute_timestamp", [](libremidi::midi_in_poll_wrapper &self) { return self.impl.absolute_timestamp(); })
      .def("poll", &libremidi::midi_in_poll_wrapper::poll);

  nb::class_<libremidi::midi_out>(m, "MidiOutBase");
  nb::class_<libremidi::midi_out_poll_wrapper>(m, "MidiOut")
      .def(nb::init<>())
      .def(nb::init<libremidi::output_configuration>())
      .def(nb::init<libremidi::output_configuration, libremidi::API>())
      .def("get_current_api", [](libremidi::midi_out_poll_wrapper &self) { return self.impl.get_current_api(); })
      .def("open_port", [](libremidi::midi_out_poll_wrapper &self, const libremidi::output_port &p) { return self.impl.open_port(p); })
      .def("open_port", [](libremidi::midi_out_poll_wrapper &self, const libremidi::output_port &p, std::string_view name) { return self.impl.open_port(p, name); })
      .def("open_virtual_port", [](libremidi::midi_out_poll_wrapper &self) { return self.impl.open_virtual_port(); })
      .def("open_virtual_port", [](libremidi::midi_out_poll_wrapper &self, std::string_view name) { return self.impl.open_virtual_port(name); })
      .def("set_port_name", [](libremidi::midi_out_poll_wrapper &self, std::string_view name) { return self.impl.set_port_name(name); })
      .def("close_port", [](libremidi::midi_out_poll_wrapper &self) { return self.impl.close_port(); })
      .def("is_port_open", [](libremidi::midi_out_poll_wrapper &self) { return self.impl.is_port_open(); })
      .def("is_port_connected", [](libremidi::midi_out_poll_wrapper &self) { return self.impl.is_port_connected(); })
      .def("absolute_timestamp", [](libremidi::midi_out_poll_wrapper &self) { return self.impl.current_time(); })

      // clang-format off
      .def("send_message", [](libremidi::midi_out_poll_wrapper &self, const libremidi::message& m) { return self.impl.send_message(m); })
      .def("send_message", [](libremidi::midi_out_poll_wrapper &self, const unsigned char* m, size_t size) { return self.impl.send_message(m, size); })
      .def("send_message", [](libremidi::midi_out_poll_wrapper &self, std::vector<unsigned char> m)  { return self.impl.send_message(m); })
      .def("send_message", [](libremidi::midi_out_poll_wrapper &self, unsigned char b0)  { return self.impl.send_message(b0); })
      .def("send_message", [](libremidi::midi_out_poll_wrapper &self, unsigned char b0, unsigned char  b1)  { return self.impl.send_message(b0, b1); })
      .def("send_message", [](libremidi::midi_out_poll_wrapper &self, unsigned char b0, unsigned char  b1, unsigned char b2)  { return self.impl.send_message(b0, b1, b2); })

      .def("schedule_message", [](libremidi::midi_out_poll_wrapper &self, int64_t t, const unsigned char* m, size_t size) { return self.impl.schedule_message(t, m, size); })

      .def("send_ump", [](libremidi::midi_out_poll_wrapper &self, const libremidi::ump& m) { return self.impl.send_ump(m); })
      .def("send_ump", [](libremidi::midi_out_poll_wrapper &self, const uint32_t* ump, size_t size) { return self.impl.send_ump(ump, size); })
      .def("send_ump", [](libremidi::midi_out_poll_wrapper &self, std::vector<uint32_t> m) { return self.impl.send_ump(m); })
      .def("send_ump", [](libremidi::midi_out_poll_wrapper &self, uint32_t u0) { return self.impl.send_ump(u0); })
      .def("send_ump", [](libremidi::midi_out_poll_wrapper &self, uint32_t u0, uint32_t u1) { return self.impl.send_ump(u0, u1); })
      .def("send_ump", [](libremidi::midi_out_poll_wrapper &self, uint32_t u0, uint32_t u1, uint32_t u2) { return self.impl.send_ump(u0, u1, u2); })
      .def("send_ump", [](libremidi::midi_out_poll_wrapper &self, uint32_t u0, uint32_t u1, uint32_t u2, uint32_t u3) { return self.impl.send_ump(u0, u1, u2, u3); })

      .def("schedule_message", [](libremidi::midi_out_poll_wrapper &self, int64_t t, const uint32_t* m, size_t size) { return self.impl.schedule_ump(t, m, size); })
      // clang-format on

      .def("poll", &libremidi::midi_out_poll_wrapper::poll);
  ;
}
