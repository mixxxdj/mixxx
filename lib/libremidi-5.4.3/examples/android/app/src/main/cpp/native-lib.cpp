#include <libremidi/libremidi.hpp>

#include <android/log.h>

#include <jni.h>

#include <sstream>
#include <string>
#include <vector>
extern "C" JNIEXPORT jstring JNICALL
Java_com_libremidi_example_MainActivity_getMidiDevices(JNIEnv* env, jobject /* this */)
{
  std::stringstream result;
  __android_log_print(
      ANDROID_LOG_WARN, "libremidi", "Java_com_libremidi_example_MainActivity_getMidiDevices!!");

  try
  {
    auto api = libremidi::API::ANDROID_AMIDI;
    libremidi::observer obs{{.track_any = true}, libremidi::observer_configuration_for(api)};

    result << "=== MIDI Input Devices ===\n";
    auto inputs = obs.get_input_ports();
    if (inputs.empty())
    {
      result << "No input devices found\n";
    }
    else
    {
      for (const auto& port : inputs)
      {
        result << "Port " << port.port << ": " << port.port_name;
        if (!port.device_name.empty() && port.device_name != port.port_name)
        {
          result << " (Device: " << port.device_name << ")";
        }
        if (!port.display_name.empty() && port.display_name != port.port_name)
        {
          result << " [" << port.display_name << "]";
        }
        result << "\n";

        __android_log_print(ANDROID_LOG_WARN, "libremidi", "opening: %s", port.port_name.c_str());
        auto midi_in = new libremidi::midi_in{
            {.on_message{[](const libremidi::message& m) {
          __android_log_print(ANDROID_LOG_WARN, "libremidi", "ayy!!");
        }}},
            libremidi::midi_in_configuration_for(api)};

        auto p = midi_in->get_current_api();

        __android_log_print(
            ANDROID_LOG_WARN, "libremidi", ">> ??! %s", libremidi::get_api_name(p).data());
        __android_log_print(ANDROID_LOG_WARN, "libremidi", ">> open port?!");
        midi_in->open_port(port);
      }
    }

    result << "\n=== MIDI Output Devices ===\n";
    auto outputs = obs.get_output_ports();
    if (outputs.empty())
    {
      result << "No output devices found\n";
    }
    else
    {
      for (const auto& port : outputs)
      {
        result << "Port " << port.port << ": " << port.port_name;
        if (!port.device_name.empty() && port.device_name != port.port_name)
        {
          result << " (Device: " << port.device_name << ")";
        }
        if (!port.display_name.empty() && port.display_name != port.port_name)
        {
          result << " [" << port.display_name << "]";
        }
        result << "\n";
      }
    }
  }
  catch (const std::exception& e)
  {
    result << "Error: " << e.what() << "\n";
  }

  __android_log_print(ANDROID_LOG_WARN, "libremidi", "FINITO!!");
  return env->NewStringUTF(result.str().c_str());
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_libremidi_example_MainActivity_getLibremidiVersion(JNIEnv* env, jobject /* this */)
{
  return env->NewStringUTF("libremidi Android example");
}