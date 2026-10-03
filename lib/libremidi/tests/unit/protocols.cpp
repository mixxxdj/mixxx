#include "../include_catch.hpp"

#include <libremidi/protocols/mmc.hpp>
#include <libremidi/protocols/msc.hpp>
#include <libremidi/protocols/mvc.hpp>

// MMC Tests

TEST_CASE("MMC message building", "[mmc]")
{
  libremidi::mmc_protocol mmc;
  mmc.device_id = 0x7F;

  SECTION("simple transport commands")
  {
    auto msg = mmc.make_command(libremidi::mmc_protocol::command_id::play);
    REQUIRE(msg.size() == 6);
    REQUIRE(msg[0] == 0xF0);
    REQUIRE(msg[1] == 0x7F);
    REQUIRE(msg[2] == 0x7F);
    REQUIRE(msg[3] == 0x06);
    REQUIRE(msg[4] == 0x02); // play
    REQUIRE(msg[5] == 0xF7);

    auto stop_msg = mmc.make_command(libremidi::mmc_protocol::command_id::stop);
    REQUIRE(stop_msg[4] == 0x01);

    auto pause_msg = mmc.make_command(libremidi::mmc_protocol::command_id::pause);
    REQUIRE(pause_msg[4] == 0x09);

    auto ff_msg = mmc.make_command(libremidi::mmc_protocol::command_id::fast_forward);
    REQUIRE(ff_msg[4] == 0x04);

    auto rw_msg = mmc.make_command(libremidi::mmc_protocol::command_id::rewind);
    REQUIRE(rw_msg[4] == 0x05);
  }

  SECTION("locate with SMPTE timecode")
  {
    auto msg = mmc.make_locate(1, 30, 45, 24, 0);
    REQUIRE(msg.size() == 13);
    REQUIRE(msg[0] == 0xF0);
    REQUIRE(msg[4] == 0x44); // locate
    REQUIRE(msg[5] == 0x06); // info field byte count
    REQUIRE(msg[6] == 0x01); // standard sub-command
    REQUIRE(msg[7] == 1);    // hr
    REQUIRE(msg[8] == 30);   // min
    REQUIRE(msg[9] == 45);   // sec
    REQUIRE(msg[10] == 24);  // frames
    REQUIRE(msg[11] == 0);   // subframes
    REQUIRE(msg[12] == 0xF7);
  }

  SECTION("specific device ID")
  {
    mmc.device_id = 0x05;
    auto msg = mmc.make_command(libremidi::mmc_protocol::command_id::play);
    REQUIRE(msg[2] == 0x05);
  }
}

TEST_CASE("MMC input parsing", "[mmc]")
{
  libremidi::mmc_protocol::command_variant received;
  bool called = false;
  libremidi::mmc_processor proc{{
      .midi_out = [](libremidi::message&&) {},
      .on_command = [&](const auto& v) { received = v; called = true; },
  }};

  SECTION("parse transport commands")
  {
    libremidi::message msg{0xF0, 0x7F, 0x7F, 0x06, 0x02, 0xF7};
    proc.on_midi(msg);
    REQUIRE(called);
    auto* play = libremidi::get_if<libremidi::mmc_protocol::play>(&received);
    REQUIRE(play != nullptr);
    REQUIRE(play->target_device == 0x7F);
  }

  SECTION("parse stop")
  {
    proc.on_midi(libremidi::message{0xF0, 0x7F, 0x7F, 0x06, 0x01, 0xF7});
    REQUIRE(libremidi::get_if<libremidi::mmc_protocol::stop>(&received) != nullptr);
  }

  SECTION("parse record strobe and exit")
  {
    proc.on_midi(libremidi::message{0xF0, 0x7F, 0x7F, 0x06, 0x06, 0xF7});
    REQUIRE(libremidi::get_if<libremidi::mmc_protocol::record_strobe>(&received) != nullptr);

    proc.on_midi(libremidi::message{0xF0, 0x7F, 0x7F, 0x06, 0x07, 0xF7});
    REQUIRE(libremidi::get_if<libremidi::mmc_protocol::record_exit>(&received) != nullptr);
  }

  SECTION("parse locate with SMPTE")
  {
    libremidi::message msg{0xF0, 0x7F, 0x7F, 0x06, 0x44, 0x06, 0x01, 10, 20, 30, 15, 0, 0xF7};
    proc.on_midi(msg);
    auto* loc = libremidi::get_if<libremidi::mmc_protocol::locate>(&received);
    REQUIRE(loc != nullptr);
    REQUIRE(loc->hr == 10);
    REQUIRE(loc->min == 20);
    REQUIRE(loc->sec == 30);
    REQUIRE(loc->frames == 15);
    REQUIRE(loc->subframes == 0);
  }

  SECTION("device ID filtering")
  {
    proc.impl.device_id = 0x05;

    // Message to device 0x05 should be accepted
    proc.on_midi(libremidi::message{0xF0, 0x7F, 0x05, 0x06, 0x02, 0xF7});
    REQUIRE(called);

    // Broadcast 0x7F should also be accepted
    called = false;
    proc.on_midi(libremidi::message{0xF0, 0x7F, 0x7F, 0x06, 0x02, 0xF7});
    REQUIRE(called);

    // Message to device 0x03 should be ignored
    called = false;
    proc.on_midi(libremidi::message{0xF0, 0x7F, 0x03, 0x06, 0x02, 0xF7});
    REQUIRE_FALSE(called);
  }

  SECTION("non-SysEx messages are ignored")
  {
    proc.on_midi(libremidi::message{0x90, 0x3C, 0x7F}); // Note On
    REQUIRE_FALSE(called);
  }

  SECTION("roundtrip: build then parse")
  {
    libremidi::mmc_protocol builder;
    builder.device_id = 0x7F;

    auto msg = builder.make_locate(2, 15, 30, 12, 3);
    proc.on_midi(msg);
    auto* loc = libremidi::get_if<libremidi::mmc_protocol::locate>(&received);
    REQUIRE(loc != nullptr);
    REQUIRE(loc->hr == 2);
    REQUIRE(loc->min == 15);
    REQUIRE(loc->sec == 30);
    REQUIRE(loc->frames == 12);
    REQUIRE(loc->subframes == 3);
  }
}

// MSC Tests

TEST_CASE("MSC command_format values match RP-002 spec", "[msc]")
{
  using cf = libremidi::msc_protocol::command_format;
  // Verify critical hex values from RP-002 spec table
  REQUIRE(static_cast<uint8_t>(cf::lighting) == 0x01);
  REQUIRE(static_cast<uint8_t>(cf::chasers) == 0x06);
  REQUIRE(static_cast<uint8_t>(cf::sound) == 0x10);
  REQUIRE(static_cast<uint8_t>(cf::machinery) == 0x20);
  REQUIRE(static_cast<uint8_t>(cf::rigging) == 0x21);
  REQUIRE(static_cast<uint8_t>(cf::video) == 0x30);
  REQUIRE(static_cast<uint8_t>(cf::projection) == 0x40);
  REQUIRE(static_cast<uint8_t>(cf::process_control) == 0x50);
  REQUIRE(static_cast<uint8_t>(cf::pyro) == 0x60);
  REQUIRE(static_cast<uint8_t>(cf::all_types) == 0x7F);
}

TEST_CASE("MSC message building", "[msc]")
{
  libremidi::msc_protocol msc;
  msc.format = libremidi::msc_protocol::command_format::lighting;

  SECTION("GO with cue number")
  {
    auto msg = msc.make_command(libremidi::msc_protocol::command_id::go, "1");
    // F0 7F 7F 02 01 01 31 F7
    REQUIRE(msg.size() == 8);
    REQUIRE(msg[3] == 0x02); // MSC sub-ID
    REQUIRE(msg[4] == 0x01); // lighting
    REQUIRE(msg[5] == 0x01); // go
    REQUIRE(msg[6] == 0x31); // ASCII '1'
    REQUIRE(msg[7] == 0xF7);
  }

  SECTION("GO with cue number and list")
  {
    auto msg = msc.make_command(libremidi::msc_protocol::command_id::go, "1.5", "2");
    // F0 7F 7F 02 01 01 31 2E 35 00 32 F7
    REQUIRE(msg.size() == 12);
    REQUIRE(msg[6] == 0x31);  // '1'
    REQUIRE(msg[7] == 0x2E);  // '.'
    REQUIRE(msg[8] == 0x35);  // '5'
    REQUIRE(msg[9] == 0x00);  // delimiter
    REQUIRE(msg[10] == 0x32); // '2'
    REQUIRE(msg[11] == 0xF7);
  }

  SECTION("command without cue data")
  {
    auto msg = msc.make_command(libremidi::msc_protocol::command_id::reset);
    REQUIRE(msg.size() == 7);
    REQUIRE(msg[5] == 0x0A); // reset
    REQUIRE(msg[6] == 0xF7);
  }

  SECTION("timed GO with time and cue")
  {
    auto msg = msc.make_timed_go(1, 2, 3, 4, 5, "10");
    // F0 7F 7F 02 01 04 01 02 03 04 05 31 30 F7
    REQUIRE(msg.size() == 14);
    REQUIRE(msg[5] == 0x04);  // timed_go
    REQUIRE(msg[6] == 1);     // hh
    REQUIRE(msg[7] == 2);     // mm
    REQUIRE(msg[8] == 3);     // ss
    REQUIRE(msg[9] == 4);     // fr
    REQUIRE(msg[10] == 5);    // ff
    REQUIRE(msg[11] == 0x31); // '1'
    REQUIRE(msg[12] == 0x30); // '0'
    REQUIRE(msg[13] == 0xF7);
  }

  SECTION("different command formats use correct hex values")
  {
    msc.format = libremidi::msc_protocol::command_format::video;
    auto msg = msc.make_command(libremidi::msc_protocol::command_id::go, "1");
    REQUIRE(msg[4] == 0x30); // video = 0x30

    msc.format = libremidi::msc_protocol::command_format::pyro;
    msg = msc.make_command(libremidi::msc_protocol::command_id::fire, "1");
    REQUIRE(msg[4] == 0x60); // pyro = 0x60
    REQUIRE(msg[5] == 0x07); // fire

    msc.format = libremidi::msc_protocol::command_format::machinery;
    msg = msc.make_command(libremidi::msc_protocol::command_id::go, "1");
    REQUIRE(msg[4] == 0x20); // machinery = 0x20

    msc.format = libremidi::msc_protocol::command_format::projection;
    msg = msc.make_command(libremidi::msc_protocol::command_id::go, "1");
    REQUIRE(msg[4] == 0x40); // projection = 0x40

    msc.format = libremidi::msc_protocol::command_format::process_control;
    msg = msc.make_command(libremidi::msc_protocol::command_id::go, "1");
    REQUIRE(msg[4] == 0x50); // process_control = 0x50
  }

  SECTION("sound commands")
  {
    msc.format = libremidi::msc_protocol::command_format::sound;
    auto msg = msc.make_command(libremidi::msc_protocol::command_id::start_clock, "1");
    REQUIRE(msg[4] == 0x10); // sound
    REQUIRE(msg[5] == 0x15); // start_clock
  }
}

TEST_CASE("MSC input parsing", "[msc]")
{
  libremidi::msc_protocol::command_variant received;
  bool called = false;
  libremidi::msc_processor proc{{
      .midi_out = [](libremidi::message&&) {},
      .on_command = [&](const auto& v) { received = v; called = true; },
  }};

  SECTION("parse GO with cue number")
  {
    // GO "1" for lighting
    proc.on_midi(libremidi::message{0xF0, 0x7F, 0x7F, 0x02, 0x01, 0x01, 0x31, 0xF7});
    auto* go = libremidi::get_if<libremidi::msc_protocol::go>(&received);
    REQUIRE(go != nullptr);
    REQUIRE(go->cue_number == "1");
    REQUIRE(go->format == libremidi::msc_protocol::command_format::lighting);
    REQUIRE(go->cue_list.empty());
    REQUIRE(go->cue_path.empty());
  }

  SECTION("parse GO with cue number and list")
  {
    proc.on_midi(libremidi::message{
        0xF0, 0x7F, 0x7F, 0x02, 0x01, 0x01, 0x31, 0x2E, 0x35, 0x00, 0x32, 0xF7});
    auto* go = libremidi::get_if<libremidi::msc_protocol::go>(&received);
    REQUIRE(go != nullptr);
    REQUIRE(go->cue_number == "1.5");
    REQUIRE(go->cue_list == "2");
  }

  SECTION("parse all general command types")
  {
    proc.on_midi(libremidi::message{0xF0, 0x7F, 0x7F, 0x02, 0x7F, 0x02, 0xF7});
    REQUIRE(libremidi::get_if<libremidi::msc_protocol::stop>(&received) != nullptr);

    proc.on_midi(libremidi::message{0xF0, 0x7F, 0x7F, 0x02, 0x7F, 0x03, 0xF7});
    REQUIRE(libremidi::get_if<libremidi::msc_protocol::resume>(&received) != nullptr);

    proc.on_midi(libremidi::message{0xF0, 0x7F, 0x7F, 0x02, 0x7F, 0x05, 0xF7});
    REQUIRE(libremidi::get_if<libremidi::msc_protocol::load>(&received) != nullptr);

    proc.on_midi(libremidi::message{0xF0, 0x7F, 0x7F, 0x02, 0x7F, 0x07, 0xF7});
    REQUIRE(libremidi::get_if<libremidi::msc_protocol::fire>(&received) != nullptr);

    proc.on_midi(libremidi::message{0xF0, 0x7F, 0x7F, 0x02, 0x7F, 0x08, 0xF7});
    REQUIRE(libremidi::get_if<libremidi::msc_protocol::all_off>(&received) != nullptr);

    proc.on_midi(libremidi::message{0xF0, 0x7F, 0x7F, 0x02, 0x7F, 0x0A, 0xF7});
    REQUIRE(libremidi::get_if<libremidi::msc_protocol::reset>(&received) != nullptr);

    proc.on_midi(libremidi::message{0xF0, 0x7F, 0x7F, 0x02, 0x7F, 0x0B, 0xF7});
    REQUIRE(libremidi::get_if<libremidi::msc_protocol::go_off>(&received) != nullptr);
  }

  SECTION("parse sound commands")
  {
    proc.on_midi(libremidi::message{0xF0, 0x7F, 0x7F, 0x02, 0x10, 0x15, 0x31, 0xF7});
    auto* sc = libremidi::get_if<libremidi::msc_protocol::start_clock>(&received);
    REQUIRE(sc != nullptr);
    REQUIRE(sc->cue_number == "1");
    REQUIRE(sc->format == libremidi::msc_protocol::command_format::sound);

    proc.on_midi(libremidi::message{0xF0, 0x7F, 0x7F, 0x02, 0x10, 0x16, 0xF7});
    REQUIRE(libremidi::get_if<libremidi::msc_protocol::stop_clock>(&received) != nullptr);

    proc.on_midi(libremidi::message{0xF0, 0x7F, 0x7F, 0x02, 0x10, 0x10, 0xF7});
    REQUIRE(libremidi::get_if<libremidi::msc_protocol::go_jam_clock>(&received) != nullptr);

    proc.on_midi(libremidi::message{0xF0, 0x7F, 0x7F, 0x02, 0x10, 0x1B, 0xF7});
    REQUIRE(libremidi::get_if<libremidi::msc_protocol::open_cue_list>(&received) != nullptr);
  }

  SECTION("parse TIMED_GO")
  {
    // TIMED_GO with time (10:20:30:15.0) and cue "1"
    proc.on_midi(
        libremidi::message{0xF0, 0x7F, 0x7F, 0x02, 0x7F, 0x04, 10, 20, 30, 15, 0, 0x31, 0xF7});
    auto* tg = libremidi::get_if<libremidi::msc_protocol::timed_go>(&received);
    REQUIRE(tg != nullptr);
    REQUIRE(tg->hours == 10);
    REQUIRE(tg->minutes == 20);
    REQUIRE(tg->seconds == 30);
    REQUIRE(tg->frames == 15);
    REQUIRE(tg->fractional_frames == 0);
    REQUIRE(tg->cue_number == "1");
  }

  SECTION("parse TIMED_GO without cue data")
  {
    proc.on_midi(
        libremidi::message{0xF0, 0x7F, 0x7F, 0x02, 0x7F, 0x04, 1, 2, 3, 4, 5, 0xF7});
    auto* tg = libremidi::get_if<libremidi::msc_protocol::timed_go>(&received);
    REQUIRE(tg != nullptr);
    REQUIRE(tg->hours == 1);
    REQUIRE(tg->cue_number.empty());
  }

  SECTION("device ID filtering")
  {
    proc.impl.device_id = 0x05;

    proc.on_midi(libremidi::message{0xF0, 0x7F, 0x05, 0x02, 0x7F, 0x01, 0xF7});
    REQUIRE(called);

    called = false;
    proc.on_midi(libremidi::message{0xF0, 0x7F, 0x03, 0x02, 0x7F, 0x01, 0xF7});
    REQUIRE_FALSE(called);
  }

  SECTION("roundtrip: build then parse")
  {
    libremidi::msc_protocol builder;
    builder.format = libremidi::msc_protocol::command_format::video;

    auto msg = builder.make_command(libremidi::msc_protocol::command_id::go, "42.5", "3");
    proc.on_midi(msg);
    auto* go = libremidi::get_if<libremidi::msc_protocol::go>(&received);
    REQUIRE(go != nullptr);
    REQUIRE(go->cue_number == "42.5");
    REQUIRE(go->cue_list == "3");
    REQUIRE(go->format == libremidi::msc_protocol::command_format::video);
  }

  SECTION("roundtrip: timed_go build then parse")
  {
    libremidi::msc_protocol builder;
    builder.format = libremidi::msc_protocol::command_format::all_types;

    auto msg = builder.make_timed_go(1, 30, 45, 24, 12, "100", "5");
    proc.on_midi(msg);
    auto* tg = libremidi::get_if<libremidi::msc_protocol::timed_go>(&received);
    REQUIRE(tg != nullptr);
    REQUIRE(tg->hours == 1);
    REQUIRE(tg->minutes == 30);
    REQUIRE(tg->seconds == 45);
    REQUIRE(tg->frames == 24);
    REQUIRE(tg->fractional_frames == 12);
    REQUIRE(tg->cue_number == "100");
    REQUIRE(tg->cue_list == "5");
  }
}

// MVC Tests

TEST_CASE("MVC control enum matches RP-050 defaults", "[mvc]")
{
  using c = libremidi::mvc_protocol::control;
  // RP-050 Section 2.2.1 default CC assignments
  REQUIRE(static_cast<uint8_t>(c::bank_select_msb) == 0);
  REQUIRE(static_cast<uint8_t>(c::dissolve_time_msb) == 5);
  REQUIRE(static_cast<uint8_t>(c::bank_select_lsb) == 32);
  REQUIRE(static_cast<uint8_t>(c::dissolve_time_lsb) == 37);
  REQUIRE(static_cast<uint8_t>(c::effect_control_1) == 71);
  REQUIRE(static_cast<uint8_t>(c::effect_control_2) == 73);
  REQUIRE(static_cast<uint8_t>(c::effect_control_3) == 74);
  REQUIRE(static_cast<uint8_t>(c::reset) == 121);
}

TEST_CASE("MVC SysEx checksum", "[mvc]")
{
  // Enable: address 10 00 00, data 01
  // Sum = 0x10 + 0x00 + 0x00 + 0x01 = 0x11
  // Checksum = (128 - 0x11) % 128 = 0x6F
  const uint8_t enable_payload[] = {0x10, 0x00, 0x00, 0x01};
  REQUIRE(libremidi::mvc_protocol::compute_checksum(enable_payload) == 0x6F);

  // Disable: address 10 00 00, data 00
  // Sum = 0x10
  // Checksum = (128 - 0x10) % 128 = 0x70
  const uint8_t disable_payload[] = {0x10, 0x00, 0x00, 0x00};
  REQUIRE(libremidi::mvc_protocol::compute_checksum(disable_payload) == 0x70);
}

TEST_CASE("MVC message building", "[mvc]")
{
  libremidi::mvc_protocol mvc;
  mvc.device_id = 0x00;

  SECTION("enable SysEx with Data Set format")
  {
    auto msg = mvc.make_enable();
    // F0 7E 00 0C 01 10 00 00 01 6F F7
    REQUIRE(msg.size() == 11);
    REQUIRE(msg[0] == 0xF0);
    REQUIRE(msg[1] == 0x7E);  // Non-Real-Time
    REQUIRE(msg[2] == 0x00);  // device ID
    REQUIRE(msg[3] == 0x0C);  // MVC sub-ID
    REQUIRE(msg[4] == 0x01);  // version 1.0
    REQUIRE(msg[5] == 0x10);  // address hi
    REQUIRE(msg[6] == 0x00);  // address mid
    REQUIRE(msg[7] == 0x00);  // address lo
    REQUIRE(msg[8] == 0x01);  // data: enable
    REQUIRE(msg[9] == 0x6F);  // checksum
    REQUIRE(msg[10] == 0xF7);
  }

  SECTION("disable SysEx with Data Set format")
  {
    auto msg = mvc.make_disable();
    REQUIRE(msg.size() == 11);
    REQUIRE(msg[8] == 0x00);  // data: disable
    REQUIRE(msg[9] == 0x70);  // checksum
  }

  SECTION("specific device ID")
  {
    mvc.device_id = 0x10;
    auto msg = mvc.make_enable();
    REQUIRE(msg[2] == 0x10);
  }
}

TEST_CASE("MVC input parsing", "[mvc]")
{
  libremidi::mvc_protocol::event_variant received;
  bool called = false;
  libremidi::mvc_processor proc{{
      .midi_out = [](libremidi::message&&) {},
      .on_event = [&](const auto& v) { received = v; called = true; },
  }};

  SECTION("parse enable SysEx with checksum")
  {
    // F0 7E 00 0C 01 10 00 00 01 6F F7
    proc.on_midi(
        libremidi::message{0xF0, 0x7E, 0x00, 0x0C, 0x01, 0x10, 0x00, 0x00, 0x01, 0x6F, 0xF7});
    auto* en = libremidi::get_if<libremidi::mvc_protocol::enable_mvc>(&received);
    REQUIRE(en != nullptr);
    REQUIRE(en->target_device == 0x00);
  }

  SECTION("parse disable SysEx")
  {
    proc.on_midi(
        libremidi::message{0xF0, 0x7E, 0x00, 0x0C, 0x01, 0x10, 0x00, 0x00, 0x00, 0x70, 0xF7});
    REQUIRE(libremidi::get_if<libremidi::mvc_protocol::disable_mvc>(&received) != nullptr);
  }

  SECTION("reject SysEx with bad checksum")
  {
    proc.on_midi(
        libremidi::message{0xF0, 0x7E, 0x00, 0x0C, 0x01, 0x10, 0x00, 0x00, 0x01, 0x00, 0xF7});
    REQUIRE_FALSE(called);
  }

  SECTION("accept SysEx without checksum (10 bytes, interop with tschiemer/midimessage)")
  {
    // F0 7E 00 0C 01 10 00 00 01 F7 (no checksum byte)
    proc.on_midi(
        libremidi::message{0xF0, 0x7E, 0x00, 0x0C, 0x01, 0x10, 0x00, 0x00, 0x01, 0xF7});
    auto* en = libremidi::get_if<libremidi::mvc_protocol::enable_mvc>(&received);
    REQUIRE(en != nullptr);
  }

  SECTION("parse Note On as clip trigger")
  {
    proc.on_midi(libremidi::message{0x90, 0x3C, 0x7F}); // ch1, note 60, vel 127
    auto* clip = libremidi::get_if<libremidi::mvc_protocol::clip_trigger>(&received);
    REQUIRE(clip != nullptr);
    REQUIRE(clip->channel == 1);
    REQUIRE(clip->note == 0x3C);
    REQUIRE(clip->pressed == true);
  }

  SECTION("parse Note On with velocity 0 as release")
  {
    proc.on_midi(libremidi::message{0x90, 0x3C, 0x00});
    auto* clip = libremidi::get_if<libremidi::mvc_protocol::clip_trigger>(&received);
    REQUIRE(clip != nullptr);
    REQUIRE(clip->pressed == false);
  }

  SECTION("parse Note Off as clip release")
  {
    proc.on_midi(libremidi::message{0x80, 0x3C, 0x40});
    auto* clip = libremidi::get_if<libremidi::mvc_protocol::clip_trigger>(&received);
    REQUIRE(clip != nullptr);
    REQUIRE(clip->pressed == false);
  }

  SECTION("parse CC as control change with raw cc number")
  {
    // CC 71 (Effect Control 1), value 100
    proc.on_midi(libremidi::message{0xB0, 71, 100});
    auto* cc = libremidi::get_if<libremidi::mvc_protocol::control_change>(&received);
    REQUIRE(cc != nullptr);
    REQUIRE(cc->channel == 1);
    REQUIRE(cc->cc == 71);
    REQUIRE(cc->value == 100);

    // CC 5 (Dissolve Time MSB)
    proc.on_midi(libremidi::message{0xB0, 5, 64});
    cc = libremidi::get_if<libremidi::mvc_protocol::control_change>(&received);
    REQUIRE(cc != nullptr);
    REQUIRE(cc->cc == static_cast<uint8_t>(libremidi::mvc_protocol::control::dissolve_time_msb));
  }

  SECTION("parse CC 121 as reset")
  {
    proc.on_midi(libremidi::message{0xB0, 121, 0});
    auto* cc = libremidi::get_if<libremidi::mvc_protocol::control_change>(&received);
    REQUIRE(cc != nullptr);
    REQUIRE(cc->cc == static_cast<uint8_t>(libremidi::mvc_protocol::control::reset));
    REQUIRE(cc->value == 0);
  }

  SECTION("parse Program Change as clip select")
  {
    proc.on_midi(libremidi::message{0xC0, 0x05});
    auto* pc = libremidi::get_if<libremidi::mvc_protocol::clip_select>(&received);
    REQUIRE(pc != nullptr);
    REQUIRE(pc->channel == 1);
    REQUIRE(pc->program == 5);
  }

  SECTION("parse Pitch Bend as playback speed")
  {
    // Center value (8192) = normal speed
    proc.on_midi(libremidi::message{0xE0, 0x00, 0x40});
    auto* pb = libremidi::get_if<libremidi::mvc_protocol::playback_speed>(&received);
    REQUIRE(pb != nullptr);
    REQUIRE(pb->channel == 1);
    REQUIRE(pb->value == 8192);
  }

  SECTION("parse Pitch Bend extremes")
  {
    proc.on_midi(libremidi::message{0xE0, 0x00, 0x00});
    auto* pb = libremidi::get_if<libremidi::mvc_protocol::playback_speed>(&received);
    REQUIRE(pb != nullptr);
    REQUIRE(pb->value == 0);

    proc.on_midi(libremidi::message{0xE0, 0x7F, 0x7F});
    pb = libremidi::get_if<libremidi::mvc_protocol::playback_speed>(&received);
    REQUIRE(pb != nullptr);
    REQUIRE(pb->value == 16383);
  }

  SECTION("SysEx device ID filtering")
  {
    proc.impl.device_id = 0x05;

    // Correct device ID
    proc.on_midi(libremidi::message{
        0xF0, 0x7E, 0x05, 0x0C, 0x01, 0x10, 0x00, 0x00, 0x01, 0x6F, 0xF7});
    REQUIRE(called);

    // Wrong device ID
    called = false;
    proc.on_midi(libremidi::message{
        0xF0, 0x7E, 0x03, 0x0C, 0x01, 0x10, 0x00, 0x00, 0x01, 0x6F, 0xF7});
    REQUIRE_FALSE(called);

    // Broadcast 0x7F should be accepted
    called = false;
    proc.on_midi(libremidi::message{
        0xF0, 0x7E, 0x7F, 0x0C, 0x01, 0x10, 0x00, 0x00, 0x01, 0x6F, 0xF7});
    REQUIRE(called);
  }

  SECTION("roundtrip: build enable then parse")
  {
    libremidi::mvc_protocol builder;
    builder.device_id = 0x00;
    auto msg = builder.make_enable();
    proc.impl.device_id = 0x00;
    proc.on_midi(msg);
    REQUIRE(libremidi::get_if<libremidi::mvc_protocol::enable_mvc>(&received) != nullptr);
  }
}

TEST_CASE("MVC processor output", "[mvc]")
{
  std::vector<libremidi::message> sent;
  libremidi::mvc_processor proc{{
      .midi_out = [&](libremidi::message&& m) { sent.push_back(std::move(m)); },
      .on_event = [](auto&&) {},
  }};

  SECTION("enable sends correct SysEx")
  {
    proc.enable();
    REQUIRE(sent.size() == 1);
    REQUIRE(sent[0].size() == 11);
    REQUIRE(sent[0][1] == 0x7E);
    REQUIRE(sent[0][4] == 0x01); // version
    REQUIRE(sent[0][8] == 0x01); // enable
  }

  SECTION("trigger_clip sends Note On")
  {
    proc.trigger_clip(1, 60, 100);
    REQUIRE(sent.size() == 1);
    REQUIRE(sent[0].get_message_type() == libremidi::message_type::NOTE_ON);
    REQUIRE(sent[0].get_channel() == 1);
    REQUIRE(sent[0][1] == 60);
    REQUIRE(sent[0][2] == 100);
  }

  SECTION("stop_clip sends Note Off")
  {
    proc.stop_clip(1, 60);
    REQUIRE(sent.size() == 1);
    REQUIRE(sent[0].get_message_type() == libremidi::message_type::NOTE_OFF);
  }

  SECTION("set_control sends CC")
  {
    proc.set_control(
        1, static_cast<uint8_t>(libremidi::mvc_protocol::control::effect_control_1), 100);
    REQUIRE(sent.size() == 1);
    REQUIRE(sent[0].get_message_type() == libremidi::message_type::CONTROL_CHANGE);
    REQUIRE(sent[0][1] == 71); // CC 71
    REQUIRE(sent[0][2] == 100);
  }

  SECTION("select_clip sends Program Change")
  {
    proc.select_clip(1, 5);
    REQUIRE(sent.size() == 1);
    REQUIRE(sent[0].get_message_type() == libremidi::message_type::PROGRAM_CHANGE);
    REQUIRE(sent[0][1] == 5);
  }

  SECTION("set_playback_speed sends Pitch Bend")
  {
    proc.set_playback_speed(1, 8192);
    REQUIRE(sent.size() == 1);
    REQUIRE(sent[0].get_message_type() == libremidi::message_type::PITCH_BEND);
  }
}
