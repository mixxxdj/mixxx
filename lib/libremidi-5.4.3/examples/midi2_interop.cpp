#include "utils.hpp"

#include <libremidi/libremidi.hpp>

#if defined(_WIN32) && __has_include(<winrt/base.h>)
  #include <winrt/base.h>
#endif

#include <midi/capability_inquiry.h>
#include <midi/midi1_byte_stream.h>
#include <midi/sysex_collector.h>
#include <midi/universal_packet.h>

#include <bitset>
#include <cstdlib>
#include <iostream>

struct midi_ci_processor
{
  midi::sysex7_collector sysex7;
  midi::sysex8_collector sysex8;

  libremidi::observer obs;
  libremidi::midi_out midiout;
  libremidi::midi_in midiin;

  midi_ci_processor()
      : sysex7{[this](const midi::sysex7& m) { on_sysex7(m); }}
      , sysex8{[this](const midi::sysex8& m, uint8_t stream_id) { on_sysex8(m, stream_id); }}
      , obs{{.track_any = true}, libremidi::midi2::observer_default_configuration()}
      , midiout{{}, libremidi::midi_out_configuration_for(obs)}
      , midiin{
            {// Set our callback function.
             .on_message = [&](const libremidi::ump& message) { on_ump(message); }},
            libremidi::midi_in_configuration_for(obs)}
  {
    sysex7.set_max_sysex_data_size(65535);
    sysex8.set_max_sysex_data_size(65535);
  }

  // 1. Create the connection
  void open()
  {
    // Opening midi input and output
    {
      auto pi = obs.get_input_ports();
      if (pi.empty())
      {
        throw "No MIDI 2 input device found\n";
      }

      for (const auto& port : pi)
      {
        std::cout << "In: " << port.port_name << " | " << port.display_name << std::endl;
      }

      midiin.open_port(pi[1]);
    }

    {
      auto po = obs.get_output_ports();
      if (po.empty())
      {
        throw "No MIDI 2 output device found\n";
      }

      for (const auto& port : po)
      {
        std::cout << "Out: " << port.port_name << " | " << port.display_name << std::endl;
      }
      midiout.open_port(po[1]);
    }
  }

  // 2. Top-level UMP processing
  void on_ump(const midi::universal_packet& pkt)
  {
    switch (pkt.type())
    {
      case midi::packet_type::utility:
        std::cerr << "[utility]" << std::endl;
        break;
      case midi::packet_type::system:
        std::cerr << "[system]" << std::endl;
        break;
      case midi::packet_type::midi1_channel_voice:
        std::cerr << "[midi1_channel_voice]" << std::endl;
        break;
      case midi::packet_type::data:
        sysex7.feed(pkt);
        break;
      case midi::packet_type::midi2_channel_voice:
        std::cerr << "[midi2_channel_voice]" << std::endl;
        break;
      case midi::packet_type::extended_data:
        std::cerr << "[sysex8]" << std::endl;
        sysex8.feed(pkt);
        break;
      case midi::packet_type::flex_data:
        std::cerr << "[flex_data]" << std::endl;
        break;
      case midi::packet_type::stream:
        std::cerr << "[stream]" << std::endl;
        break;
      default:
        break;
    }
  }

  // 3. These two callbacks are called when sysex7 / sysex8 are fed enough
  void on_sysex7(const midi::sysex7& sx)
  {
    auto v = midi::as_universal_sysex_view(sx);
    if (!v)
      return;
    on_sysex(*v);
  }

  void on_sysex8(const midi::sysex8&, uint8_t stream_id) { }

  // 4. Actual message processing
  void on_sysex(const midi::universal_sysex_view& us)
  {
    using namespace midi::universal_sysex::type;
    switch (us.type())
    {
      case sample_dump_header:
        std::cerr << "[sysex] sample_dump_header" << std::endl;
        break;
      case sample_data_packet:
        std::cerr << "[sysex] sample_data_packet" << std::endl;
        break;
      case sample_dump_request:
        std::cerr << "[sysex] sample_dump_request" << std::endl;
        break;
      case midi_time_code_non_real_time:
        std::cerr << "[sysex] midi_time_code_non_real_time" << std::endl;
        break;
      case sample_dump_extensions:
        std::cerr << "[sysex] sample_dump_extensions" << std::endl;
        break;
      case general_information:
        std::cerr << "[sysex] general_information" << std::endl;
        break;
      case file_dump:
        std::cerr << "[sysex] file_dump" << std::endl;
        break;
      case midi_tuning_non_real_time:
        std::cerr << "[sysex] midi_tuning_non_real_time" << std::endl;
        break;
      case general_midi:
        std::cerr << "[sysex] general_midi" << std::endl;
        break;
      case downloadable_sounds:
        std::cerr << "[sysex] downloadable_sounds" << std::endl;
        break;
      case file_reference_message:
        std::cerr << "[sysex] file_reference_message" << std::endl;
        break;
      case midi_visual_control:
        std::cerr << "[sysex] midi_visual_control" << std::endl;
        break;
      case capability_inquiry:
        std::cerr << "[sysex] ";
        on_midi_ci(midi::capability_inquiry_view{us});
        break;
      case end_of_file:
        std::cerr << "[sysex] end_of_file" << std::endl;
        break;
      case wait:
        std::cerr << "[sysex] wait" << std::endl;
        break;
      case cancel:
        std::cerr << "[sysex] cancel" << std::endl;
        break;
      case nak:
        std::cerr << "[sysex] nak" << std::endl;
        break;
      case ack:
        std::cerr << "[sysex] ack" << std::endl;
        break;

        // real time (7fh)
      case midi_time_code_real_time:
        std::cerr << "[sysex] midi_time_code_real_time" << std::endl;
        break;
      case midi_show_control:
        std::cerr << "[sysex] midi_show_control" << std::endl;
        break;
      case notation_information:
        std::cerr << "[sysex] notation_information" << std::endl;
        break;
      case device_control:
        std::cerr << "[sysex] device_control" << std::endl;
        break;
      case real_time_mtc_cueing:
        std::cerr << "[sysex] real_time_mtc_cueing" << std::endl;
        break;
      case midi_machine_control_commands:
        std::cerr << "[sysex] midi_machine_control_commands" << std::endl;
        break;
      case midi_machine_control_responses:
        std::cerr << "[sysex] midi_machine_control_responses" << std::endl;
        break;
      case midi_tuning_real_time:
        std::cerr << "[sysex] midi_tuning_real_time" << std::endl;
        break;
      case controller_destination_setting:
        std::cerr << "[sysex] controller_destination_setting" << std::endl;
        break;
        break;
      default:
        std::cerr << "[sysex] UNKNOWN" << std::endl;
        break;
    }
  }

  // MIDI-CI processing
  void on_midi_ci(const midi::capability_inquiry_view& ci)
  {
    using namespace midi::ci::subtype;
    switch (ci.subtype())
    {
      /// CI discovery ///
      case discovery_inquiry: {
        std::cerr << "[ci.management] discovery_inquiry\n";
        auto v = midi::ci::discovery_inquiry_view{ci};

        std::cerr << " - Source muid: " << v.source_muid() << "\n";
        std::cerr << " - Dst muid: " << v.destination_muid() << "\n";
        std::cerr << " - ident: " << v.identity().manufacturer << "\n";
        std::cerr << " - Supported categories: " << std::bitset<8>(v.categories()) << "\n";
        break;
      }
      case discovery_reply: {
        std::cerr << "[ci.management] discovery_reply\n";
        auto v = midi::ci::discovery_reply_view{ci};

        std::cerr << " - Src muid: " << v.source_muid() << "\n";
        std::cerr << " - Dst muid: " << v.destination_muid() << "\n";
        std::cerr << " - ident: " << v.identity().manufacturer << "\n";
        std::cerr << " - Supported categories: " << std::bitset<8>(v.categories()) << "\n";
        break;
      }
      case endpoint_information_inquiry:
        std::cerr << "[ci.management] endpoint_information_inquiry" << std::endl;
        break;
      case endpoint_information_reply:
        std::cerr << "[ci.management] endpoint_information_reply" << std::endl;
        break;
      case ack:
        std::cerr << "[ci.management] ack" << std::endl;
        break;
      case invalidate_muid:
        std::cerr << "[ci.management] invalidate_muid" << std::endl;
        break;
      case nak:
        std::cerr << "[ci.management] nak" << std::endl;
        break;

        /// Profile inquiry ///
      case profile_inquiry:
        std::cerr << "[ci.pi] profile inquiry" << std::endl;
        break;
      case profile_inquiry_reply:
        std::cerr << "[ci.pi] profile inquiry reply" << std::endl;
        break;
      case set_profile_on:
        std::cerr << "[ci.pi] set_profile_on" << std::endl;
        break;
      case set_profile_off:
        std::cerr << "[ci.pi] set_profile_off" << std::endl;
        break;
      case profile_enabled:
        std::cerr << "[ci.pi] profile_enabled" << std::endl;
        break;
      case profile_disabled:
        std::cerr << "[ci.pi] profile_disabled" << std::endl;
        break;
      case profile_added:
        std::cerr << "[ci.pi] profile_added" << std::endl;
        break;
      case profile_removed:
        std::cerr << "[ci.pi] profile_removed" << std::endl;
        break;
      case profile_details_inquiry:
        std::cerr << "[ci.pi] profile_details_inquiry" << std::endl;
        break;
      case profile_details_reply:
        std::cerr << "[ci.pi] profile_details_reply" << std::endl;
        break;
      case profile_specific_data:
        std::cerr << "[ci.pi] profile_specific_data" << std::endl;
        break;

        // Property exchange
      case property_exchange_capabilities_inquiry:
        std::cerr << "[ci.pex] property_exchange_capabilities_inquiry" << std::endl;
        break;
      case property_exchange_capabilities_reply:
        std::cerr << "[ci.pex] property_exchange_capabilities_reply" << std::endl;
        break;
      case _pe_reserved_1_:
        std::cerr << "[ci.pex] _pe_reserved_1_" << std::endl;
        break;
      case _pe_reserved_2_:
        std::cerr << "[ci.pex] _pe_reserved_2_" << std::endl;
        break;
      case get_property_data_inquiry:
        std::cerr << "[ci.pex] get_property_data_inquiry" << std::endl;
        break;
      case get_property_data_reply:
        std::cerr << "[ci.pex] get_property_data_reply" << std::endl;
        break;
      case set_property_data_inquiry:
        std::cerr << "[ci.pex] set_property_data_inquiry" << std::endl;
        break;
      case set_property_data_reply:
        std::cerr << "[ci.pex] set_property_data_reply" << std::endl;
        break;
      case subscription_inquiry:
        std::cerr << "[ci.pex] subscription_inquiry" << std::endl;
        break;
      case subscription_reply:
        std::cerr << "[ci.pex] subscription_reply" << std::endl;
        break;
      case notify:
        std::cerr << "[ci.pex] notify" << std::endl;
        break;
    }
  }
};

int main()
try
{
#if defined(_WIN32) && __has_include(<winrt/base.h>)
  // Necessary for using WinUWP and WinMIDI, must be done as early as possible in your main()
  winrt::init_apartment();
#endif

  midi_ci_processor processor;

  processor.open();
  midi::muid_t my_muid = rand() % 0xFFFFEFF;
  std::cerr << "muid: " << my_muid << "\n";
  midi::ci::identity_t id{
      .manufacturer = midi::manufacturer::atari,
      .family = 0x0102,
      .model = 0x0304,
      .revision = 0x00000506};

  for (;;)
  {
    // 128 minimum, 512 for property exchange as per MIDI-CI v1.2 5.5.3
    auto inquiry = midi::ci::make_discovery_inquiry(my_muid, id, 0x02, 512);

    processor.midiout.send_ump(inquiry);
    sleep(2);
  }

  char input;
  std::cin.get(input);
}
catch (const std::exception& error)
{
  std::cerr << error.what() << std::endl;
  return EXIT_FAILURE;
}
