#pragma once
#include <libremidi/config.hpp>

#include <functional>
#include <map>

NAMESPACE_LIBREMIDI
{
enum kbd_event
{
  NOTE_0 = 0x0,         // C
  VEL_0 = NOTE_0 + 128, // Set velocity to 0
  OCT_0 = VEL_0 + 128,  // Set octave to 0
  OCTAVE_PLUS = OCT_0 + 128,
  OCTAVE_MINUS,
  VELOCITY_PLUS,
  VELOCITY_MINUS,
};

// Default map
//
// ,---,---,---,---,---,---,---,---,---,---,---,---,---,-------,
// | V0| V1| V2| V3| V4| V5| V6| V7| V8| V9|V10|V11|V12| <-    |
// |---'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-----|
// | ->| |   | C#| D#|   | F#| G#| A#|   | C#| D#|   | F#|     |
// |-----',--',--',--',--',--',--',--',--',--',--',--',--'|    |
// | Caps | C | D | E | F | G | A | B | C | D | E | F | G |    |
// |----,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'---'----|
// | -^ |   | O-| O+| V-| V+|   |   |   |   |   |   |   ----^  |
// |----'-,-',--'--,'---'---'---'---'---'---'-,-'---',--,------|
// | ctrl |  | alt |                          |altgr |  | ctrl |
// '------'  '-----'--------------------------'------'  '------'

// clang-format off

// actually based on macOS virtual codes as it seems
// impossible to get raw scan codes.
// https://eastmanreference.com/complete-list-of-applescript-key-codes
LIBREMIDI_STATIC const std::map<int, int>& scancode_map_macos(){
  static const std::map<int, int> ret{
  {  0, NOTE_0}, // C0
  { 13, NOTE_0 + 1},
  {  1, NOTE_0 + 2},
  { 14, NOTE_0 + 3},
  {  2, NOTE_0 + 4},
  {  3, NOTE_0 + 5},
  { 17, NOTE_0 + 6},
  {  5, NOTE_0 + 7},
  { 16, NOTE_0 + 8},
  {  4, NOTE_0 + 9},
  { 32, NOTE_0 + 10},
  { 38, NOTE_0 + 11},
  { 40, NOTE_0 + 12}, // C1
  { 31, NOTE_0 + 13},
  { 37, NOTE_0 + 14},
  { 35, NOTE_0 + 15},
  { 41, NOTE_0 + 16},
  { 39, NOTE_0 + 17},
  { 30, NOTE_0 + 18},
  { 42, NOTE_0 + 19},

  { 50, VEL_0 + int(0)},
  { 18, VEL_0 + int(1 * 127 / 12)},
  { 19, VEL_0 + int(2 * 127 / 12)},
  { 20, VEL_0 + int(3 * 127 / 12)},
  { 21, VEL_0 + int(4 * 127 / 12)},
  { 23, VEL_0 + int(5 * 127 / 12)},
  { 22, VEL_0 + int(6 * 127 / 12)},
  { 26, VEL_0 + int(7 * 127 / 12)},
  { 28, VEL_0 + int(8 * 127 / 12)},
  { 25, VEL_0 + int(9 * 127 / 12)},
  { 29, VEL_0 + int(10 * 127 / 12)},
  { 27, VEL_0 + int(11 * 127 / 12)},
  { 24, VEL_0 + int(127)},

  { 6, OCTAVE_MINUS },
  { 7, OCTAVE_PLUS },
  { 8, VELOCITY_MINUS },
  { 9, VELOCITY_PLUS},
  };
  return ret;
}

// Use: https://kbdlayout.info/KBDUSX/scancodes
// Also valid for windows
LIBREMIDI_STATIC const std::map<int, int>& scancode_map_linux() {
  static const std::map<int, int> ret{
  { 0x1E, NOTE_0}, // C0
  { 0x11, NOTE_0 + 1},
  { 0x1F, NOTE_0 + 2},
  { 0x12, NOTE_0 + 3},
  { 0x20, NOTE_0 + 4},
  { 0x21, NOTE_0 + 5},
  { 0x14, NOTE_0 + 6},
  { 0x22, NOTE_0 + 7},
  { 0x15, NOTE_0 + 8},
  { 0x23, NOTE_0 + 9},
  { 0x16, NOTE_0 + 10},
  { 0x24, NOTE_0 + 11},
  { 0x25, NOTE_0 + 12}, // C1
  { 0x18, NOTE_0 + 13},
  { 0x26, NOTE_0 + 14},
  { 0x19, NOTE_0 + 15},
  { 0x27, NOTE_0 + 16},
  { 0x28, NOTE_0 + 17},
  { 0x1B, NOTE_0 + 18},
  { 0x2B, NOTE_0 + 19},

  { 0x29, VEL_0 + int(0)},
  { 0x02, VEL_0 + int(1 * 127 / 12)},
  { 0x03, VEL_0 + int(2 * 127 / 12)},
  { 0x04, VEL_0 + int(3 * 127 / 12)},
  { 0x05, VEL_0 + int(4 * 127 / 12)},
  { 0x06, VEL_0 + int(5 * 127 / 12)},
  { 0x07, VEL_0 + int(6 * 127 / 12)},
  { 0x08, VEL_0 + int(7 * 127 / 12)},
  { 0x09, VEL_0 + int(8 * 127 / 12)},
  { 0x0A, VEL_0 + int(9 * 127 / 12)},
  { 0x0B, VEL_0 + int(10 * 127 / 12)},
  { 0x0C, VEL_0 + int(11 * 127 / 12)},
  { 0x0D, VEL_0 + int(127)},

  { 0x2C, OCTAVE_MINUS },
  { 0x2D, OCTAVE_PLUS },
  { 0x2E, VELOCITY_MINUS },
  { 0x2F, VELOCITY_PLUS},
  };
  return ret;
}
// clang-format on
/**
 * Used to set up keyboard input.
 * Your app should pass a function that will give you a callback
 * that you should call whenever you are getting a key input (press or release).
 */
struct kbd_input_configuration
{
  // Note that on Linux with X11 scancodes should be offset by 8 (an X11 legacy quirk)
  // ex.: pass QKeyEvent::nativeScanCode() - 8 to this API
  using scancode_callback = std::function<void(int)>;

  // First argument is on key press, second on key release
  std::function<void(scancode_callback, scancode_callback)> set_input_scancode_callbacks
      = [](const scancode_callback&, const scancode_callback&) { };

  std::map<int, int> scancode_map
#if defined(__APPLE__)
      = scancode_map_macos()
#else
      = scancode_map_linux()
#endif
      ;
};
}
