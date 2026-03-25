#include <libremidi/reader.hpp>

#if defined(_WIN32) && __has_include(<winrt/base.h>)
  #include <winrt/base.h>
#endif

#include <cmath>

#include <fstream>
#include <iomanip>
#include <iostream>
#include <vector>

int main(int argc, char** argv)
{
#if defined(_WIN32) && __has_include(<winrt/base.h>)
  // Necessary for using WinUWP and WinMIDI, must be done as early as possible in your main()
  winrt::init_apartment();
#endif

  if (argc < 2)
  {
    perror("Usage: ./midifile_dump <midifile.mid>");
    return 1;
  }

  // Read raw from a MIDI file
  std::ifstream file{argv[1], std::ios::binary};
  if (!file.is_open())
  {
    std::cerr << "Could not open " << argv[1] << std::endl;
    return 1;
  }

  std::vector<uint8_t> bytes;
  bytes.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());

  // Initialize our reader object
  libremidi::reader r{true};

  // Parse
  libremidi::reader::parse_result result = r.parse(bytes);

  switch (result)
  {
    case libremidi::reader::validated:
      std::cout << "\nParsing validated\n\n";
      // Parsing has succeeded, all the input data is correct MIDI.
      break;

    case libremidi::reader::complete:
      std::cout << "\nParsing complete\n\n";
      // All the input data is parsed but the MIDI file was not necessarily strict SMF
      // (e.g. there are empty tracks or tracks without END OF TRACK)
      break;

    case libremidi::reader::incomplete:
      std::cout << "\nParsing incomplete\n\n";
      // Not all the input could be parsed. For instance a track could not be read.
      break;

    case libremidi::reader::invalid:
      std::cout << "\nParsing invalid\n\n";
      // Nothing could be parsed, this is not MIDI data or we do not support it yet.
      return 1;
  }

  if (result != libremidi::reader::invalid)
  {
    long beatDuration = (long)(60. * 1'000'000. / r.startingTempo);    // [usecs]
    long tickDuration = beatDuration / r.ticksPerBeat;                 // [usecs]
    long duration = (long)r.get_end_time() * tickDuration / 1'000'000; // [secs]
    std::cout << r.ticksPerBeat << " ticks per beat, ";
    std::cout << "Tick: " << tickDuration << " usecs, "
              << "Tempo: " << 60 * 1'000'000 / beatDuration << " bpm "
              << "Duration: " << duration / 60 << ":" << std::setw(2) << std::setfill('0')
              << duration % 60;

    for (const auto& track : r.tracks)
    {
      std::cout << "\nNew track\n\n";
      for (const libremidi::track_event& event : track)
      {
        std::cout << "Event at " << event.tick << " : ";
        if (event.m.is_meta_event())
        {
          std::cout << "Meta event ";
          switch (event.m.get_meta_event_type())
          {
            case libremidi::meta_event_type::SEQUENCE_NUMBER:
              std::cout << "SEQUENCE_NUMBER "
                        << (((uint16_t)event.m.bytes[3]) << 8) + event.m.bytes[4];
              break;
            case libremidi::meta_event_type::TEXT:
              std::cout << "TEXT: ";
              for (int i = 3; i < event.m.bytes.size(); ++i)
                std::cout << event.m.bytes[i];
              break;
            case libremidi::meta_event_type::COPYRIGHT:
              std::cout << "COPYRIGHT: ";
              for (int i = 3; i < event.m.bytes.size(); ++i)
                std::cout << event.m.bytes[i];
              break;
            case libremidi::meta_event_type::TRACK_NAME:
              std::cout << "TRACK_NAME: ";
              for (int i = 3; i < event.m.bytes.size(); ++i)
                std::cout << event.m.bytes[i];
              break;
            case libremidi::meta_event_type::INSTRUMENT:
              std::cout << "INSTRUMENT: ";
              for (int i = 3; i < event.m.bytes.size(); ++i)
                std::cout << event.m.bytes[i];
              break;
            case libremidi::meta_event_type::LYRIC:
              std::cout << "LYRIC: ";
              for (int i = 3; i < event.m.bytes.size(); ++i)
                std::cout << event.m.bytes[i];
              break;
            case libremidi::meta_event_type::MARKER:
              std::cout << "MARKER: ";
              for (int i = 3; i < event.m.bytes.size(); ++i)
                std::cout << event.m.bytes[i];
              break;
            case libremidi::meta_event_type::CUE:
              std::cout << "CUE: ";
              for (int i = 3; i < event.m.bytes.size(); ++i)
                std::cout << event.m.bytes[i];
              break;
            case libremidi::meta_event_type::PATCH_NAME:
              std::cout << "PATCH_NAME: ";
              for (int i = 3; i < event.m.bytes.size(); ++i)
                std::cout << event.m.bytes[i];
              break;
            case libremidi::meta_event_type::DEVICE_NAME:
              std::cout << "DEVICE_NAME: ";
              for (int i = 3; i < event.m.bytes.size(); ++i)
                std::cout << event.m.bytes[i];
              break;
            case libremidi::meta_event_type::CHANNEL_PREFIX:
              std::cout << "CHANNEL_PREFIX " << (uint16_t)event.m.bytes[3];
              break;
            case libremidi::meta_event_type::MIDI_PORT:
              std::cout << "MIDI_PORT " << (uint16_t)event.m.bytes[3];
              break;
            case libremidi::meta_event_type::END_OF_TRACK:
              std::cout << "END_OF_TRACK";
              break;
            case libremidi::meta_event_type::TEMPO_CHANGE:
              std::cout << "TEMPO_CHANGE: " << std::dec
                        << ((((uint32_t)event.m.bytes[3]) << 16) + (event.m.bytes[4] << 8)
                            + event.m.bytes[5])
                        << " usec / beat, ";
              beatDuration
                  = ((((uint32_t)event.m.bytes[3]) << 16) + (event.m.bytes[4] << 8)
                     + event.m.bytes[5]); // [usecs]
              tickDuration = beatDuration / r.ticksPerBeat;
              duration = (long)r.get_end_time() * tickDuration / 1'000'000;
              std::cout << "Tick: " << tickDuration << " usecs, "
                        << "Tempo: " << 60 * 1'000'000 / beatDuration << " bpm "
                        << "Duration: " << duration / 60 << ":" << std::setw(2)
                        << std::setfill('0') << duration % 60;
              break;
            case libremidi::meta_event_type::SMPTE_OFFSET:
              std::cout << "SMPTE_OFFSET";
              break;
            case libremidi::meta_event_type::TIME_SIGNATURE:
              std::cout << "TIME_SIGNATURE: " << (int)event.m.bytes[3] << "/"
                        << (int)std::pow(2., event.m.bytes[4]) << ", " << (int)event.m.bytes[5]
                        << " clocks per beat, " << (int)event.m.bytes[6]
                        << " 32nd notes per quarter note";
              break;
            case libremidi::meta_event_type::KEY_SIGNATURE:
              std::cout << "KEY_SIGNATURE: ";
              if (event.m.bytes[4] == 0x00)
              {
                switch (event.m.bytes[3])
                {
                  case 0xf9:
                    std::cout << "C sharp ";
                    break;
                  case 0xfa:
                    std::cout << "F sharp ";
                    break;
                  case 0xfb:
                    std::cout << "B ";
                    break;
                  case 0xfc:
                    std::cout << "E ";
                    break;
                  case 0xfd:
                    std::cout << "A ";
                    break;
                  case 0xfe:
                    std::cout << "D ";
                    break;
                  case 0xff:
                    std::cout << "G ";
                    break;
                  case 0x00:
                    std::cout << "C ";
                    break;
                  case 0x01:
                    std::cout << "F ";
                    break;
                  case 0x02:
                    std::cout << "B flat ";
                    break;
                  case 0x03:
                    std::cout << "E flat ";
                    break;
                  case 0x04:
                    std::cout << "A flat ";
                    break;
                  case 0x05:
                    std::cout << "D flat ";
                    break;
                  case 0x06:
                    std::cout << "G flat ";
                    break;
                  case 0x07:
                    std::cout << "C flat ";
                    break;
                  default:
                    std::cout << "unknown " << (int)event.m.bytes[3] << " ";
                }
                std::cout << "Major";
              }
              else if (event.m.bytes[4] == 0x01)
              {
                switch (event.m.bytes[3])
                {
                  case 0xf9:
                    std::cout << "a flat ";
                    break;
                  case 0xfa:
                    std::cout << "e flat ";
                    break;
                  case 0xfb:
                    std::cout << "b flat ";
                    break;
                  case 0xfc:
                    std::cout << "f ";
                    break;
                  case 0xfd:
                    std::cout << "c ";
                    break;
                  case 0xfe:
                    std::cout << "g ";
                    break;
                  case 0xff:
                    std::cout << "d ";
                    break;
                  case 0x00:
                    std::cout << "a ";
                    break;
                  case 0x01:
                    std::cout << "e ";
                    break;
                  case 0x02:
                    std::cout << "b ";
                    break;
                  case 0x03:
                    std::cout << "f sharp ";
                    break;
                  case 0x04:
                    std::cout << "c sharp ";
                    break;
                  case 0x05:
                    std::cout << "g sharp ";
                    break;
                  case 0x06:
                    std::cout << "d sharp ";
                    break;
                  case 0x07:
                    std::cout << "a sharp ";
                    break;
                  default:
                    std::cout << "unknown key " << (int)event.m.bytes[3];
                }
                std::cout << "Minor";
              }
              else
                std::cout << "unknown mode " << (int)event.m.bytes[3];
              break;
            case libremidi::meta_event_type::PROPRIETARY:
              std::cout << "PROPRIETARY";
              break;
            case libremidi::meta_event_type::UNKNOWN:
              std::cout << "UNKNOWN";
              break;
            default:
              std::cout << "Unsupported.";
              break;
          }
        }
        else
        {
          switch (event.m.get_message_type())
          {
            case libremidi::message_type::NOTE_ON:
              std::cout << "Note ON: "
                        << "channel " << event.m.get_channel() << ' ' << "note "
                        << (int)event.m.bytes[1] << ' ' << "velocity " << (int)event.m.bytes[2]
                        << ' ';
              break;
            case libremidi::message_type::NOTE_OFF:
              std::cout << "Note OFF: "
                        << "channel " << event.m.get_channel() << ' ' << "note "
                        << (int)event.m.bytes[1] << ' ' << "velocity " << (int)event.m.bytes[2]
                        << ' ';
              break;
            case libremidi::message_type::CONTROL_CHANGE:
              std::cout << "Control: "
                        << "channel " << event.m.get_channel() << ' ' << "control "
                        << (int)event.m.bytes[1] << ' ' << "value " << (int)event.m.bytes[2]
                        << ' ';
              break;
            case libremidi::message_type::PROGRAM_CHANGE:
              std::cout << "Program: "
                        << "channel " << event.m.get_channel() << ' ' << "program "
                        << (int)event.m.bytes[1] << ' ';
              break;
            case libremidi::message_type::AFTERTOUCH:
              std::cout << "Aftertouch: "
                        << "channel " << event.m.get_channel() << ' ' << "value "
                        << (int)event.m.bytes[1] << ' ';
              break;
            case libremidi::message_type::POLY_PRESSURE:
              std::cout << "Poly pressure: "
                        << "channel " << event.m.get_channel() << ' ' << "note "
                        << (int)event.m.bytes[1] << ' ' << "value " << (int)event.m.bytes[2]
                        << ' ';
              break;
            case libremidi::message_type::PITCH_BEND:
              std::cout << "Poly pressure: "
                        << "channel " << event.m.get_channel() << ' ' << "bend "
                        << (int)(event.m.bytes[1] << 7 + event.m.bytes[2]) << ' ';
              break;
            default:
              std::cout << "Unsupported.";
              break;
          }
        }
        std::cout << '\n';
      }
    }
  }
}
