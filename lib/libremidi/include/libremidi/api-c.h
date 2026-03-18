#pragma once
#ifndef LIBREMIDI_API_C_H
#define LIBREMIDI_API_C_H

#if __cplusplus && !defined(LIBREMIDI_MODULE_BUILD)
extern "C" {
#endif

//! MIDI API specifier arguments.
//! To get information on which feature is supported by each back-end, check their backend file
//! in e.g. backends/winmm.hpp, etc.
enum libremidi_api
{
  UNSPECIFIED = 0x0, /*!< Search for a working compiled API. */

  // MIDI 1.0 APIs
  COREMIDI = 0x1, /*!< macOS CoreMidi API. */
  ALSA_SEQ,       /*!< Linux ALSA Sequencer API. */
  ALSA_RAW,       /*!< Linux Raw ALSA API. */
  JACK_MIDI,      /*!< JACK Low-Latency MIDI Server API. */
  WINDOWS_MM,     /*!< Microsoft Multimedia MIDI API. */
  WINDOWS_UWP,    /*!< Microsoft WinRT MIDI API. */
  WEBMIDI,        /*!< Web MIDI API through Emscripten */
  PIPEWIRE,       /*!< PipeWire */
  KEYBOARD,       /*!< Computer keyboard input */
  NETWORK,        /*!< MIDI over IP */
  ANDROID_AMIDI,  /*!< Android AMidi API */
  KDMAPI,         /*!< OmniMIDI KDMAPI (Windows) */

  // MIDI 2.0 APIs
  ALSA_RAW_UMP = 0x1000, /*!< Raw ALSA API for MIDI 2.0 */
  ALSA_SEQ_UMP,          /*!< Linux ALSA Sequencer API for MIDI 2.0 */
  COREMIDI_UMP,          /*!< macOS CoreMidi API for MIDI 2.0. Requires macOS 11+ */
  WINDOWS_MIDI_SERVICES, /*!< Windows API for MIDI 2.0. Requires Windows 11 */
  KEYBOARD_UMP,          /*!< Computer keyboard input */
  NETWORK_UMP,           /*!< MIDI2 over IP */
  JACK_UMP,              /*!< MIDI2 over JACK, type "32 bit raw UMP". Requires PipeWire v1.4+. */
  PIPEWIRE_UMP,          /*!< MIDI2 over PipeWire. Requires v1.4+. */

  DUMMY = 0xFFFF /*!< A compilable but non-functional API. */
};

typedef enum libremidi_api libremidi_api;

#if __cplusplus && !defined(LIBREMIDI_MODULE_BUILD)
}
#endif
#endif
