#include "utils.hpp"

#include <libremidi/backends/linux/pipewire.hpp>
#include <libremidi/configurations.hpp>
#include <libremidi/detail/memory.hpp>
#include <libremidi/libremidi.hpp>

#include <jack/jack.h>

#include <chrono>
#include <thread>

auto& pw = libremidi::libpipewire::instance();

struct my_app
{
  libremidi::unique_handle<pw_main_loop, [](auto ptr) { pw.main_loop_destroy(ptr); }> handle;
  pw_filter* filter{};

  std::optional<libremidi::observer> observer;

  std::vector<libremidi::midi_in> midiin;
  std::vector<libremidi::midi_out> midiout;

  std::vector<libremidi::pipewire_callback> midiin_callbacks;
  std::vector<libremidi::pipewire_callback> midiout_callbacks;

  my_app()
  {
    auto callback = [&](int port, const libremidi::message& message) {
      std::cerr << port << ": " << message << std::endl;

      midiout[port].send_message(message);
    };

    // Create a PipeWire main loop which will be shared across objects
    handle.reset(pw.main_loop_new(nullptr));
    if (!handle)
      throw std::runtime_error("Could not initialize PipeWire");

    auto main_loop = handle.get();
    // Create a PipeWire filter
    {
      // clang-format off
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
      static constexpr struct pw_filter_events filter_events
          = {.version = PW_VERSION_FILTER_EVENTS,
             .process = &my_app::pipewire_callback};
#pragma GCC diagnostic pop

       filter = pw.filter_new_simple(
          pw.main_loop_get_loop(main_loop),
          "My filter",
          pw.properties_new(
              PW_KEY_MEDIA_TYPE, "Midi",
              PW_KEY_MEDIA_CATEGORY, "Filter",
              PW_KEY_MEDIA_ROLE, "DSP",
              PW_KEY_MEDIA_NAME, "libremidi",
              PW_KEY_NODE_ALWAYS_PROCESS, "true",
              PW_KEY_NODE_PAUSE_ON_IDLE, "false",
              nullptr),
          &filter_events,
          this);
      // clang-format on
      if (pw.filter_connect(this->filter, PW_FILTER_FLAG_RT_PROCESS, NULL, 0) < 0)
      {
        throw std::runtime_error("Could not connect PipeWire filter");
      }
    }

    // Create an observer
    observer = libremidi::observer{
        libremidi::observer_configuration{},
        libremidi::pipewire_observer_configuration{.context = handle.get()}};

    // Create our configuration
    auto api_input_config = libremidi::pipewire_input_configuration{
        .context = main_loop,
        .filter = filter,
        .set_process_func
        = [this](libremidi::pipewire_callback cb) { midiin_callbacks.push_back(std::move(cb)); }};
    auto api_output_config = libremidi::pipewire_output_configuration{
        .context = main_loop,
        .filter = filter,
        .set_process_func
        = [this](libremidi::pipewire_callback cb) { midiout_callbacks.push_back(std::move(cb)); }};

    // Create 16 inputs and 16 outputs
    for (int i = 0; i < 16; i++)
    {
      midiin.emplace_back(
          libremidi::input_configuration{
              .on_message = [=](const libremidi::message& msg) { callback(i, msg); }},
          api_input_config);
      midiin[i].open_virtual_port("Input: " + std::to_string(i));

      midiout.emplace_back(libremidi::output_configuration{}, api_output_config);
      midiout[i].open_virtual_port("Output: " + std::to_string(i));
    }
  }

  ~my_app() { pw.filter_destroy(filter); }

  void run() { pw.main_loop_run(handle.get()); }

  static void pipewire_callback(void* ctx, spa_io_position* pos)
  {
    auto& self = *(my_app*)ctx;

    // Process the midi inputs
    for (auto& cb : self.midiin_callbacks)
      cb.callback(pos);

    // Do some other things

    // Process the midi outputs
    for (auto& cb : self.midiout_callbacks)
      cb.callback(pos);
  }
};

int main(int argc, char** argv)
{
  pw.init(&argc, &argv);
  try
  {
    my_app app{};

    app.run();
  }
  catch (...)
  {
  }
  pw.deinit();
}
