//*****************************************//
//  midi1_out_kdmapi.cpp
//
//  Example demonstrating KDMAPI (OmniMIDI) output.
//  Shows high-throughput MIDI output with KDMAPI's
//  low-latency direct data path.
//
//*****************************************//

#include <libremidi/backends/kdmapi.hpp>
#include <libremidi/libremidi.hpp>

#include <chrono>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <random>
#include <thread>

int main(int argc, char** argv)
{
  using namespace std::chrono_literals;

  // Check if KDMAPI is available
  if (!libremidi::kdmapi::kdmapi_loader::instance().is_available())
  {
    std::cerr << "KDMAPI is not available. Please install OmniMIDI.\n";
    std::cerr << "Download from: https://github.com/KeppySoftware/OmniMIDI\n";
    return 1;
  }

  std::cout << "KDMAPI is available!\n";

  // Get KDMAPI version
  auto& loader = libremidi::kdmapi::kdmapi_loader::instance();
  if (loader.ReturnKDMAPIVer)
  {
    DWORD major{}, minor{}, build{}, rev{};
    if (loader.ReturnKDMAPIVer(&major, &minor, &build, &rev))
    {
      std::cout << "KDMAPI Version: " << major << "." << minor << "." << build << " Rev. " << rev
                << "\n";
    }
  }

  // Parse arguments
  bool no_buffer = false;
  bool stress_test = false;
  int note_count = 100;

  for (int i = 1; i < argc; ++i)
  {
    std::string arg = argv[i];
    if (arg == "-n" || arg == "--no-buffer")
    {
      no_buffer = true;
    }
    else if (arg == "-s" || arg == "--stress")
    {
      stress_test = true;
    }
    else if (arg == "-c" && i + 1 < argc)
    {
      note_count = std::stoi(argv[++i]);
    }
  }

  // Configure KDMAPI output
  libremidi::kdmapi::output_configuration kdm_conf;
  kdm_conf.use_no_buffer = no_buffer;

  std::cout << "Mode: " << (no_buffer ? "No-buffer (lowest latency)" : "Buffered") << "\n";

  // Create MIDI output with KDMAPI configuration
  libremidi::midi_out midiout{{}, kdm_conf};

  // Get available ports
  libremidi::observer obs{{}, libremidi::kdmapi::observer_configuration{}};
  auto ports = obs.get_output_ports();

  if (ports.empty())
  {
    std::cerr << "No KDMAPI output ports found!\n";
    return 1;
  }

  std::cout << "Output port: " << ports[0].display_name << "\n\n";

  // Open the port
  auto err = midiout.open_port(ports[0]);
  if (err != stdx::error{})
  {
    std::cerr << "Failed to open KDMAPI port!\n";
    return 1;
  }

  if (stress_test)
  {
    // Stress test: blast as many notes as possible
    std::cout << "Running stress test with " << note_count << " notes...\n";

    std::random_device rd;
    std::mt19937 gen(rd());
    for(int i = 0; i < 1000; i++)
    {
      std::uniform_int_distribution<> note_dist(36, 96);
      std::uniform_int_distribution<> vel_dist(60, 127);
      std::uniform_int_distribution<> ch_dist(0, 15);

      auto start = std::chrono::steady_clock::now();

      for (int i = 0; i < note_count; ++i)
      {
        int note = note_dist(gen);
        int vel = vel_dist(gen);
        int ch = ch_dist(gen);

        // Note On
        midiout.send_message(0x90 | ch, note, vel);

        // Brief delay to let some notes sound
        if (i % 100 == 0)
        {
          std::this_thread::sleep_for(1ms);
        }
      }

      auto end = std::chrono::steady_clock::now();
      auto duration = std::chrono::duration<double, std::milli>(end - start).count();

      std::cout << "Sent " << note_count << " notes in " << std::fixed << std::setprecision(2)
                << duration << " ms\n";
      std::cout << "Rate: " << std::fixed << std::setprecision(0)
                << (note_count / duration * 1000.0) << " notes/sec\n";

      // Wait a bit then silence all
      std::this_thread::sleep_for(1ms);

      // All notes off on all channels
      for (int ch = 0; ch < 16; ++ch)
      {
        midiout.send_message(0xB0 | ch, 123, 0);
        midiout.send_message(0xB0 | ch, 120, 0);
      }
    }
  }
  else
  {
    // Simple demo: play a chord progression
    std::cout << "Playing a simple chord progression...\n\n";

    // Set piano on all channels
    for (int ch = 0; ch < 16; ++ch)
    {
      midiout.send_message(0xC0 | ch, 0); // Program change: Piano
    }

    // Define some chords (C major, F major, G major, C major)
    std::vector<std::vector<int>> chords = {
        {60, 64, 67},       // C major
        {60, 65, 69},       // F major
        {59, 62, 67},       // G major
        {60, 64, 67, 72},   // C major (with octave)
    };

    for (const auto& chord : chords)
    {
      // Note On for all notes in chord
      for (int note : chord)
      {
        midiout.send_message(0x90, note, 100);
        std::this_thread::sleep_for(5ms); // Slight strum effect
      }

      std::this_thread::sleep_for(500ms);

      // Note Off for all notes
      for (int note : chord)
      {
        midiout.send_message(0x80, note, 0);
      }

      std::this_thread::sleep_for(100ms);
    }

    std::cout << "Done!\n";
  }

  return 0;
}
