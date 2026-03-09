
#ifndef CMIDI2_H_INCLUDED
#define CMIDI2_H_INCLUDED

#include <libremidi/config.hpp>

#include <memory.h>
#include <stdbool.h>
#include <stdint.h>

#include <libremidi/config.hpp>

#if !defined(_MSC_VER)
#pragma GCC system_header
#pragma clang system_header
#endif

#define CMIDI2_MIDI_2_0_RESERVED 0
#define CMIDI2_JR_TIMESTAMP_TICKS_PER_SECOND 31250
// FIXME: remove those global defs that are quite harmful (conflicts with other libraries)
#define MIDI_2_0_RESERVED 0
#define JR_TIMESTAMP_TICKS_PER_SECOND 31250

#ifdef __cplusplus
extern "C" {
#endif

enum cmidi2_status_code
{
  CMIDI2_STATUS_NOTE_OFF = 0x80,
  CMIDI2_STATUS_NOTE_ON = 0x90,
  CMIDI2_STATUS_PAF = 0xA0,
  CMIDI2_STATUS_CC = 0xB0,
  CMIDI2_STATUS_PROGRAM = 0xC0,
  CMIDI2_STATUS_CAF = 0xD0,
  CMIDI2_STATUS_PITCH_BEND = 0xE0,
  CMIDI2_STATUS_PER_NOTE_RCC = 0x00,
  CMIDI2_STATUS_PER_NOTE_ACC = 0x10,
  CMIDI2_STATUS_RPN = 0x20,
  CMIDI2_STATUS_NRPN = 0x30,
  CMIDI2_STATUS_RELATIVE_RPN = 0x40,
  CMIDI2_STATUS_RELATIVE_NRPN = 0x50,
  CMIDI2_STATUS_PER_NOTE_PITCH_BEND = 0x60,
  CMIDI2_STATUS_PER_NOTE_MANAGEMENT = 0xF0,
};

enum cmidi2_message_type
{
  CMIDI2_MESSAGE_TYPE_UTILITY = 0,
  CMIDI2_MESSAGE_TYPE_SYSTEM = 1,
  CMIDI2_MESSAGE_TYPE_MIDI_1_CHANNEL = 2,
  CMIDI2_MESSAGE_TYPE_SYSEX7 = 3,
  CMIDI2_MESSAGE_TYPE_MIDI_2_CHANNEL = 4,
  CMIDI2_MESSAGE_TYPE_SYSEX8_MDS = 5,
  CMIDI2_MESSAGE_TYPE_FLEX_DATA = 0xD,
  CMIDI2_MESSAGE_TYPE_UMP_STREAM = 0xF,
};

enum cmidi2_ci_protocol_bytes
{
  CMIDI2_PROTOCOL_BYTES_TYPE = 1,
  CMIDI2_PROTOCOL_BYTES_VERSION = 2,
  CMIDI2_PROTOCOL_BYTES_EXTENSIONS = 3,
};

enum cmidi2_ci_protocol_values
{
  CMIDI2_PROTOCOL_TYPE_MIDI1 = 1,
  CMIDI2_PROTOCOL_TYPE_MIDI2 = 2,
  CMIDI2_PROTOCOL_VERSION_MIDI1 = 0,
  CMIDI2_PROTOCOL_VERSION_MIDI2_V1 = 0,
};

enum cmidi2_ci_protocol_extensions
{
  CMIDI2_PROTOCOL_EXTENSIONS_JITTER = 1,
  CMIDI2_PROTOCOL_EXTENSIONS_LARGER = 2, // only for MIDI 1.0 compat UMP
};

typedef struct cmidi2_ci_protocol_tag
{
  uint8_t protocol_type; // cmidi2_ci_protocol_bytes
  uint8_t version;       // cmidi2_ci_protocol_values
  uint8_t extensions;    // cmidi2_ci_protocol_extensions (flags)
  uint8_t reserved1;
  uint8_t reserved2;
} cmidi2_ci_protocol;

enum cmidi2_cc
{
  CMIDI2_CC_BANK_SELECT = 0x00,
  CMIDI2_CC_MODULATION = 0x01,
  CMIDI2_CC_BREATH = 0x02,
  CMIDI2_CC_FOOT = 0x04,
  CMIDI2_CC_PORTAMENTO_TIME = 0x05,
  CMIDI2_CC_DTE_MSB = 0x06,
  CMIDI2_CC_VOLUME = 0x07,
  CMIDI2_CC_BALANCE = 0x08,
  CMIDI2_CC_PAN = 0x0A,
  CMIDI2_CC_EXPRESSION = 0x0B,
  CMIDI2_CC_EFFECT_CONTROL_1 = 0x0C,
  CMIDI2_CC_EFFECT_CONTROL_2 = 0x0D,
  CMIDI2_CC_GENERAL_1 = 0x10,
  CMIDI2_CC_GENERAL_2 = 0x11,
  CMIDI2_CC_GENERAL_3 = 0x12,
  CMIDI2_CC_GENERAL_4 = 0x13,
  CMIDI2_CC_BANK_SELECT_LSB = 0x20,
  CMIDI2_CC_MODULATION_LSB = 0x21,
  CMIDI2_CC_BREATH_LSB = 0x22,
  CMIDI2_CC_FOOT_LSB = 0x24,
  CMIDI2_CC_PORTAMENTO_TIME_LSB = 0x25,
  CMIDI2_CC_DTE_LSB = 0x26,
  CMIDI2_CC_VOLUME_LSB = 0x27,
  CMIDI2_CC_BALANCE_LSB = 0x28,
  CMIDI2_CC_PAN_LSB = 0x2A,
  CMIDI2_CC_EXPRESSION_LSB = 0x2B,
  CMIDI2_CC_EFFECT1_LSB = 0x2C,
  CMIDI2_CC_EFFECT2_LSB = 0x2D,
  CMIDI2_CC_GENERAL_1_LSB = 0x30,
  CMIDI2_CC_GENERAL_2_LSB = 0x31,
  CMIDI2_CC_GENERAL_3_LSB = 0x32,
  CMIDI2_CC_GENERAL_4_LSB = 0x33,
  CMIDI2_CC_HOLD = 0x40,
  CMIDI2_CC_PORTAMENTO_SWITCH = 0x41,
  CMIDI2_CC_SOSTENUTO = 0x42,
  CMIDI2_CC_SOFT_PEDAL = 0x43,
  CMIDI2_CC_LEGATO = 0x44,
  CMIDI2_CC_HOLD_2 = 0x45,
  CMIDI2_CC_SOUND_CONTROLLER_1 = 0x46,
  CMIDI2_CC_SOUND_CONTROLLER_2 = 0x47,
  CMIDI2_CC_SOUND_CONTROLLER_3 = 0x48,
  CMIDI2_CC_SOUND_CONTROLLER_4 = 0x49,
  CMIDI2_CC_SOUND_CONTROLLER_5 = 0x4A,
  CMIDI2_CC_SOUND_CONTROLLER_6 = 0x4B,
  CMIDI2_CC_SOUND_CONTROLLER_7 = 0x4C,
  CMIDI2_CC_SOUND_CONTROLLER_8 = 0x4D,
  CMIDI2_CC_SOUND_CONTROLLER_9 = 0x4E,
  CMIDI2_CC_SOUND_CONTROLLER_10 = 0x4F,
  CMIDI2_CC_GENERAL_5 = 0x50,
  CMIDI2_CC_GENERAL_6 = 0x51,
  CMIDI2_CC_GENERAL_7 = 0x52,
  CMIDI2_CC_GENERAL_8 = 0x53,
  CMIDI2_CC_PORTAMENTO_CONTROL = 0x54,
  CMIDI2_CC_RSD = 0x5B,
  CMIDI2_CC_EFFECT_1 = 0x5B,
  CMIDI2_CC_TREMOLO = 0x5C,
  CMIDI2_CC_EFFECT_2 = 0x5C,
  CMIDI2_CC_CSD = 0x5D,
  CMIDI2_CC_EFFECT_3 = 0x5D,
  CMIDI2_CC_CELESTE = 0x5E,
  CMIDI2_CC_EFFECT_4 = 0x5E,
  CMIDI2_CC_PHASER = 0x5F,
  CMIDI2_CC_EFFECT_5 = 0x5F,
  CMIDI2_CC_DTE_INCREMENT = 0x60,
  CMIDI2_CC_DTE_DECREMENT = 0x61,
  CMIDI2_CC_NRPN_LSB = 0x62,
  CMIDI2_CC_NRPN_MSB = 0x63,
  CMIDI2_CC_RPN_LSB = 0x64,
  CMIDI2_CC_RPN_MSB = 0x65,
  // CHANNEL MODE MESSAGES
  CMIDI2_CC_ALL_SOUND_OFF = 0x78,
  CMIDI2_CC_RESET_ALL_CONTROLLERS = 0x79,
  CMIDI2_CC_LOCAL_CONTROL = 0x7A,
  CMIDI2_CC_ALL_NOTES_OFF = 0x7B,
  CMIDI2_CC_OMNI_MODE_OFF = 0x7C,
  CMIDI2_CC_OMNI_MODE_ON = 0x7D,
  CMIDI2_CC_POLY_MODE_ON_OFF = 0x7E,
  CMIDI2_CC_POLY_MODE_ON = 0x7F,
};

enum cmidi2_rpn
{
  CMIDI2_RPN_PITCH_BEND_SENSITIVITY = 0,
  CMIDI2_RPN_FINE_TUNING = 1,
  CMIDI2_RPN_COARSE_TUNING = 2,
  CMIDI2_RPN_TUNING_PROGRAM = 3,
  CMIDI2_RPN_TUNING_BANK_SELECT = 4,
  CMIDI2_RPN_MODULATION_DEPTH = 5,
};

enum cmidi2_meta_event_type
{
  CMIDI2_META_SEQUENCE_NUMBER = 0X00,
  CMIDI2_META_TEXT = 0X01,
  CMIDI2_META_COPYRIGHT = 0X02,
  CMIDI2_META_TRACK_NAME = 0X03,
  CMIDI2_META_INSTRUMENT_NAME = 0X04,
  CMIDI2_META_LYRIC = 0X05,
  CMIDI2_META_MARKER = 0X06,
  CMIDI2_META_CUE = 0X07,
  CMIDI2_META_CHANNEL_PREFIX = 0X20,
  CMIDI2_META_END_OF_TRACK = 0X2F,
  CMIDI2_META_TEMPO = 0X51,
  CMIDI2_META_SMPTE_OFFSET = 0X54,
  CMIDI2_META_TIME_SIGNATURE = 0X58,
  CMIDI2_META_KEY_SIGNATURE = 0X59,
  CMIDI2_META_SEQUENCER_SPECIFIC = 0X7F,
};

enum cmidi2_per_note_management_flags
{
  CMIDI2_PER_NOTE_MANAGEMENT_RESET = 1,
  CMIDI2_PER_NOTE_MANAGEMENT_DETACH = 2,
};

enum cmidi2_note_attribute_type
{
  CMIDI2_ATTRIBUTE_TYPE_NONE = 0,
  CMIDI2_ATTRIBUTE_TYPE_MANUFACTURER = 1,
  CMIDI2_ATTRIBUTE_TYPE_PROFILE = 2,
  CMIDI2_ATTRIBUTE_TYPE_PITCH7_9 = 3,
};

enum cmidi2_program_change_option_flags
{
  CMIDI2_PROGRAM_CHANGE_OPTION_NONE = 0,
  CMIDI2_PROGRAM_CHANGE_OPTION_BANK_VALID = 1,
};

enum cmidi2_sysex_status
{
  CMIDI2_SYSEX_IN_ONE_UMP = 0,
  CMIDI2_SYSEX_START = 0x10,
  CMIDI2_SYSEX_CONTINUE = 0x20,
  CMIDI2_SYSEX_END = 0x30,
};

enum cmidi2_mixed_data_set_status
{
  CMIDI2_MIXED_DATA_STATUS_HEADER = 0x80,
  CMIDI2_MIXED_DATA_STATUS_PAYLOAD = 0x90,
};

enum cmidi2_system_message_status
{
  CMIDI2_SYSTEM_STATUS_MIDI_TIME_CODE = 0xF1,
  CMIDI2_SYSTEM_STATUS_SONG_POSITION = 0xF2,
  CMIDI2_SYSTEM_STATUS_SONG_SELECT = 0xF3,
  CMIDI2_SYSTEM_STATUS_TUNE_REQUEST = 0xF6,
  CMIDI2_SYSTEM_STATUS_TIMING_CLOCK = 0xF8,
  CMIDI2_SYSTEM_STATUS_START = 0xFA,
  CMIDI2_SYSTEM_STATUS_CONTINUE = 0xFB,
  CMIDI2_SYSTEM_STATUS_STOP = 0xFC,
  CMIDI2_SYSTEM_STATUS_ACTIVE_SENSING = 0xFE,
  CMIDI2_SYSTEM_STATUS_RESET = 0xFF,
};

enum cmidi2_utility_message_status
{
  CMIDI2_UTILITY_STATUS_NOOP = 0,
  CMIDI2_UTILITY_STATUS_JR_CLOCK = 0x10,
  CMIDI2_UTILITY_STATUS_JR_TIMESTAMP = 0x20,
  CMIDI2_UTILITY_STATUS_DCTPQ = 0x30,
  CMIDI2_UTILITY_STATUS_DELTA_CLOCKSTAMP = 0x40,
};

enum cmidi2_flex_data_status_bank
{
  CMIDI2_FLEX_DATA_BANK_SETUP_AND_PERFORMANCE = 0,
  CMIDI2_FLEX_DATA_BANK_METADATA_TEXT = 1,
  CMIDI2_FLEX_DATA_BANK_PERFORMANCE_TEXT = 2
};
enum cmidi2_flex_data_setup_status
{
  CMIDI2_FLEX_DATA_STATUS_SET_TEMPO = 0,
  CMIDI2_FLEX_DATA_STATUS_SET_TIME_SIGNATURE = 1,
  CMIDI2_FLEX_DATA_STATUS_SET_METRONOME = 2,
  CMIDI2_FLEX_DATA_STATUS_SET_KEY_SIGNATURE = 5,
  CMIDI2_FLEX_DATA_STATUS_SET_CHORD_NAME = 6
};
enum cmidi2_flex_data_metadata_text_status
{
  CMIDI2_FLEX_DATA_STATUS_UNKNOWN_METADATA_TEXT = 0,
  CMIDI2_FLEX_DATA_STATUS_PROJECT_NAME = 1,
  CMIDI2_FLEX_DATA_STATUS_SONG_NAME = 2,
  CMIDI2_FLEX_DATA_STATUS_CLIP_NAME = 3,
  CMIDI2_FLEX_DATA_STATUS_COPYRIGHT_NAME = 4,
  CMIDI2_FLEX_DATA_STATUS_COMPOSER_NAME = 5,
  CMIDI2_FLEX_DATA_STATUS_LYRICIST_NAME = 6,
  CMIDI2_FLEX_DATA_STATUS_ARRANGER_NAME = 7,
  CMIDI2_FLEX_DATA_STATUS_PUBLISHER_NAME = 8,
  CMIDI2_FLEX_DATA_STATUS_PRIMARY_PERFORMER_NAME = 9,
  CMIDI2_FLEX_DATA_STATUS_ACCOMPANYING_PERFORMER_NAME = 10,
  CMIDI2_FLEX_DATA_STATUS_RECORDING_DATE = 11,
  CMIDI2_FLEX_DATA_STATUS_RECORDING_LOCATION = 12
};
enum cmidi2_flex_data_performance_text_status
{
  CMIDI2_FLEX_DATA_STATUS_UNKNOWN_PERFORMANCE_TEXT = 0,
  CMIDI2_FLEX_DATA_STATUS_LYRICS = 1,
  CMIDI2_FLEX_DATA_STATUS_LYRICS_LANGUAGE = 2,
  CMIDI2_FLEX_DATA_STATUS_RUBY = 3,
  CMIDI2_FLEX_DATA_STATUS_RUBY_LANGUAGE = 4
};

enum cmidi2_ump_chord_name_sharps_flats
{
  CMIDI2_UMP_CHORD_NAME_DOUBLE_SHARP = 2,
  CMIDI2_UMP_CHORD_NAME_SHARP = 1,
  CMIDI2_UMP_CHORD_NAME_NATURAL = 0,
  CMIDI2_UMP_CHORD_NAME_FLAT = 0xF,
  CMIDI2_UMP_CHORD_NAME_DOUBLE_FLAT = 0xE,
};

enum cmidi2_ump_chord_name_tonic_note
{
  CMIDI2_UMP_CHORD_NAME_UNKNOWN = 0,
  CMIDI2_UMP_CHORD_NAME_A = 1,
  CMIDI2_UMP_CHORD_NAME_B = 2,
  CMIDI2_UMP_CHORD_NAME_C = 3,
  CMIDI2_UMP_CHORD_NAME_D = 4,
  CMIDI2_UMP_CHORD_NAME_E = 5,
  CMIDI2_UMP_CHORD_NAME_F = 6,
  CMIDI2_UMP_CHORD_NAME_G = 7,
};

enum cmidi2_ump_chord_name_chord_type
{
  CMIDI2_UMP_CHORD_TYPE_UNKNOWN = 0,
  CMIDI2_UMP_CHORD_TYPE_MAJOR = 1,
  CMIDI2_UMP_CHORD_TYPE_MAJOR_6TH = 2,
  CMIDI2_UMP_CHORD_TYPE_MAJOR_7TH = 3,
  CMIDI2_UMP_CHORD_TYPE_MAJOR_9TH = 4,
  CMIDI2_UMP_CHORD_TYPE_MAJOR_11TH = 5,
  CMIDI2_UMP_CHORD_TYPE_MAJOR_13TH = 6,
  CMIDI2_UMP_CHORD_TYPE_MINOR = 7,
  CMIDI2_UMP_CHORD_TYPE_MINOR_6TH = 8,
  CMIDI2_UMP_CHORD_TYPE_MINOR_7TH = 9,
  CMIDI2_UMP_CHORD_TYPE_MINOR_9TH = 10,
  CMIDI2_UMP_CHORD_TYPE_MINOR_11TH = 11,
  CMIDI2_UMP_CHORD_TYPE_MINOR_13TH = 12,
  CMIDI2_UMP_CHORD_TYPE_DOMINANT = 13,
  CMIDI2_UMP_CHORD_TYPE_DOMINANT_9TH = 14,
  CMIDI2_UMP_CHORD_TYPE_DOMINANT_11TH = 15,
  CMIDI2_UMP_CHORD_TYPE_DOMINANT_13TH = 16,
  CMIDI2_UMP_CHORD_TYPE_AUGMENTED = 17,
  CMIDI2_UMP_CHORD_TYPE_AUGMENTED_7TH = 18,
  CMIDI2_UMP_CHORD_TYPE_DIMINISHED = 19,
  CMIDI2_UMP_CHORD_TYPE_DIMINISHED_7TH = 20,
  CMIDI2_UMP_CHORD_TYPE_HALF_DIMINISHED = 21,
  CMIDI2_UMP_CHORD_TYPE_MAJOR_MINOR = 22,
  CMIDI2_UMP_CHORD_TYPE_MINOR_MAJOR = 22, // same
  CMIDI2_UMP_CHORD_TYPE_PEDAL = 23,
  CMIDI2_UMP_CHORD_TYPE_POWER = 24,
  CMIDI2_UMP_CHORD_TYPE_SUSPENDED_2ND = 25,
  CMIDI2_UMP_CHORD_TYPE_SUSPENDED_4TH = 26,
  CMIDI2_UMP_CHORD_TYPE_7_SUSPENDED_4TH = 27,
};

LIBREMIDI_STATIC uint8_t cmidi2_ump_get_num_bytes(uint32_t data)
{
  switch (((data & 0xF0000000) >> 28) & 0xF)
  {
    case CMIDI2_MESSAGE_TYPE_UTILITY:
    case CMIDI2_MESSAGE_TYPE_SYSTEM:
    case CMIDI2_MESSAGE_TYPE_MIDI_1_CHANNEL:
      return 4;
    case CMIDI2_MESSAGE_TYPE_MIDI_2_CHANNEL:
    case CMIDI2_MESSAGE_TYPE_SYSEX7:
      return 8;
    case CMIDI2_MESSAGE_TYPE_SYSEX8_MDS:
      return 16;
  }
  return 0xFF; /* wrong */
}

typedef struct cmidi2_ump128
{
  uint32_t p1;
  uint32_t p2;
  uint32_t p3;
  uint32_t p4;
} cmidi2_ump128_t;

typedef struct cmidi2_ump_version
{
  uint8_t major;
  uint8_t minor;
} cmidi2_ump_version_t;

enum cmidi2_ump_stream_status
{
  CMIDI2_UMP_STREAM_STATUS_ENDPOINT_DISCOVERY = 0,
  CMIDI2_UMP_STREAM_STATUS_ENDPOINT_INFO = 1,
  CMIDI2_UMP_STREAM_STATUS_DEVICE_IDENTITY = 2,
  CMIDI2_UMP_STREAM_STATUS_ENDPOINT_NAME = 3,
  CMIDI2_UMP_STREAM_STATUS_PRODUCT_INSTANCE_ID = 4,
  CMIDI2_UMP_STREAM_STATUS_STREAM_CONFIGURATION_REQUEST = 5,
  CMIDI2_UMP_STREAM_STATUS_STREAM_CONFIGURATION_NOTIFICATION = 6,
  CMIDI2_UMP_STREAM_STATUS_FUNCTION_BLOCK_DISCOVERY = 0x10,
  CMIDI2_UMP_STREAM_STATUS_FUNCTION_BLOCK_INFO = 0x11,
  CMIDI2_UMP_STREAM_STATUS_FUNCTION_BLOCK_NAME = 0x12,
  CMIDI2_UMP_STREAM_STATUS_START_OF_CLIP = 0x20,
  CMIDI2_UMP_STREAM_STATUS_END_OF_CLIP = 0x21,
};

enum cmidi2_ump_endpoint_filter_flags
{
  CMIDI2_UMP_ENDPOINT_FILTER_ENDPOINT_INFO = 1,
  CMIDI2_UMP_ENDPOINT_FILTER_DEVICE_IDENTITY = 2,
  CMIDI2_UMP_ENDPOINT_FILTER_ENDPOINT_NAME = 4,
  CMIDI2_UMP_ENDPOINT_FILTER_PRODUCT_INSTANCE_ID = 8,
  CMIDI2_UMP_ENDPOINT_FILTER_STREAM_CONFIGURATION = 0x10,
};

enum cmidi2_ump_function_block_discovery_flags
{
  CMIDI2_UMP_FUNCTION_BLOCK_FILTER_INFO = 1,
  CMIDI2_UMP_FUNCTION_BLOCK_FILTER_NAME = 2,
};

// --------
// UMP generators

// 7.1 UMP Stream Messages

LIBREMIDI_STATIC cmidi2_ump128_t
cmidi2_ump_endpoint_discovery(cmidi2_ump_version_t version, uint8_t filterBitmap)
{
  cmidi2_ump128_t ret
      = {(uint32_t)((CMIDI2_MESSAGE_TYPE_UMP_STREAM << 28)
                    + (CMIDI2_UMP_STREAM_STATUS_ENDPOINT_DISCOVERY << 16) + (version.major << 8)
                    + version.minor),
         (uint32_t)(filterBitmap & 0x1F), 0, 0};
  return ret;
}

LIBREMIDI_STATIC cmidi2_ump128_t cmidi2_ump_endpoint_info_notification(
    cmidi2_ump_version_t version, bool isStaticFunctionBlock, uint8_t numFunctionBlocks,
    bool midi2Capable, bool midi1Capable, bool rxJR, bool txJR)
{
  cmidi2_ump128_t ret = {
      (uint32_t)(CMIDI2_MESSAGE_TYPE_UMP_STREAM << 28)
          + (CMIDI2_UMP_STREAM_STATUS_ENDPOINT_INFO << 16) + (version.major << 8) + version.minor,
      (uint32_t)(isStaticFunctionBlock ? 1 << 31 : 0) + (numFunctionBlocks << 24)
          + (midi2Capable ? 0x1000 : 0) + (midi1Capable ? 0x100 : 0) + (rxJR ? 2 : 0)
          + (txJR ? 1 : 0),
      0, 0};
  return ret;
}

LIBREMIDI_STATIC cmidi2_ump128_t cmidi2_ump_device_identity_notification(
    uint32_t manufacturerIdIn7bitArray, uint8_t deviceFamilyLSB, uint8_t deviceFamilyMSB,
    uint8_t deviceFamilyModelLSB, uint8_t deviceFamilyModelMSB,
    uint32_t softwareRevisionIn7bitArray)
{
  (void)deviceFamilyModelMSB;
  cmidi2_ump128_t ret
      = {(uint32_t)(CMIDI2_MESSAGE_TYPE_UMP_STREAM << 28)
             + (CMIDI2_UMP_STREAM_STATUS_DEVICE_IDENTITY << 16),
         manufacturerIdIn7bitArray,
         (uint32_t)(deviceFamilyLSB << 24) + (deviceFamilyMSB << 16) + (deviceFamilyModelLSB << 8)
             + deviceFamilyMSB,
         softwareRevisionIn7bitArray};
  return ret;
}

LIBREMIDI_STATIC cmidi2_ump128_t
cmidi2_ump_internal_name_notification(uint8_t statusCode, const char name[14])
{
  cmidi2_ump128_t ret
      = {(uint32_t)(CMIDI2_MESSAGE_TYPE_UMP_STREAM << 28) + (statusCode << 16) + (name[0] << 8)
             + name[1],
         (uint32_t)(name[2] << 24) + (name[3] << 16) + (name[4] << 8) + name[5],
         (uint32_t)(name[6] << 24) + (name[7] << 16) + (name[8] << 8) + name[9],
         (uint32_t)(name[10] << 24) + (name[11] << 16) + (name[12] << 8) + name[13]};
  return ret;
}

LIBREMIDI_STATIC cmidi2_ump128_t cmidi2_ump_endpoint_name_notification(const char name[14])
{
  return cmidi2_ump_internal_name_notification(CMIDI2_UMP_STREAM_STATUS_ENDPOINT_NAME, name);
}

LIBREMIDI_STATIC cmidi2_ump128_t cmidi2_ump_product_instance_id_notification(const char id[14])
{
  return cmidi2_ump_internal_name_notification(CMIDI2_UMP_STREAM_STATUS_PRODUCT_INSTANCE_ID, id);
}

LIBREMIDI_STATIC cmidi2_ump128_t
cmidi2_ump_stream_configuration_request(uint8_t protocol, bool rxJR, bool txJR)
{
  cmidi2_ump128_t ret
      = {(uint32_t)(CMIDI2_MESSAGE_TYPE_UMP_STREAM << 28)
             + (CMIDI2_UMP_STREAM_STATUS_STREAM_CONFIGURATION_REQUEST << 16) + (protocol << 8)
             + (rxJR ? 2 : 0) + (txJR ? 1 : 0),
         0, 0, 0};
  return ret;
}

LIBREMIDI_STATIC cmidi2_ump128_t
cmidi2_ump_stream_configuration_notification(uint8_t protocol, bool rxJR, bool txJR)
{
  cmidi2_ump128_t ret
      = {(uint32_t)(CMIDI2_MESSAGE_TYPE_UMP_STREAM << 28)
             + (CMIDI2_UMP_STREAM_STATUS_STREAM_CONFIGURATION_NOTIFICATION << 16) + (protocol << 8)
             + (rxJR ? 2 : 0) + (txJR ? 1 : 0),
         0, 0, 0};
  return ret;
}

LIBREMIDI_STATIC cmidi2_ump128_t
cmidi2_ump_function_block_discovery(uint8_t numFunctionBlocks, uint8_t filter)
{
  cmidi2_ump128_t ret
      = {(uint32_t)(CMIDI2_MESSAGE_TYPE_UMP_STREAM << 28)
             + (CMIDI2_UMP_STREAM_STATUS_FUNCTION_BLOCK_DISCOVERY << 16) + (numFunctionBlocks << 8)
             + filter,
         0, 0, 0};
  return ret;
}

LIBREMIDI_STATIC cmidi2_ump128_t cmidi2_ump_function_block_info_notification(
    bool active, uint8_t numFunctionBlocks, uint8_t uiHint, uint8_t midi1, uint8_t direction,
    uint8_t firstGroup, uint8_t numSpannedGroup, uint8_t midiCIVersionFormat,
    uint8_t maxNumSysEx8Streams)
{
  cmidi2_ump128_t ret
      = {(uint32_t)(CMIDI2_MESSAGE_TYPE_UMP_STREAM << 28)
             + (CMIDI2_UMP_STREAM_STATUS_FUNCTION_BLOCK_INFO << 16) + (active ? 0x8000 : 0)
             + (numFunctionBlocks << 8) + (uiHint << 4) + (midi1 << 2) + direction,
         (uint32_t)(firstGroup << 24) + (numSpannedGroup << 16) + (midiCIVersionFormat << 8)
             + maxNumSysEx8Streams,
         0, 0};
  return ret;
}

LIBREMIDI_STATIC cmidi2_ump128_t cmidi2_ump_function_block_name_notification(const char name[14])
{
  return cmidi2_ump_internal_name_notification(CMIDI2_UMP_STREAM_STATUS_FUNCTION_BLOCK_NAME, name);
}

LIBREMIDI_STATIC cmidi2_ump128_t cmidi2_ump_start_of_clip()
{
  cmidi2_ump128_t ret
      = {(uint32_t)(CMIDI2_MESSAGE_TYPE_UMP_STREAM << 28)
             + (CMIDI2_UMP_STREAM_STATUS_START_OF_CLIP << 16),
         0, 0, 0};
  return ret;
}

LIBREMIDI_STATIC cmidi2_ump128_t cmidi2_ump_end_of_clip()
{
  cmidi2_ump128_t ret
      = {(uint32_t)(CMIDI2_MESSAGE_TYPE_UMP_STREAM << 28)
             + (CMIDI2_UMP_STREAM_STATUS_END_OF_CLIP << 16),
         0, 0, 0};
  return ret;
}

// 7.2 Utility Messages
LIBREMIDI_STATIC uint32_t cmidi2_ump_noop()
{
  return 0;
}

LIBREMIDI_STATIC uint32_t cmidi2_ump_jr_clock_direct(uint16_t senderClockTime)
{
  return (CMIDI2_UTILITY_STATUS_JR_CLOCK << 16) + senderClockTime;
}

LIBREMIDI_STATIC uint32_t cmidi2_ump_jr_clock(double senderClockTime)
{
  uint16_t value = (uint16_t)(senderClockTime * JR_TIMESTAMP_TICKS_PER_SECOND);
  return (CMIDI2_UTILITY_STATUS_JR_CLOCK << 16) + value;
}

LIBREMIDI_STATIC uint32_t cmidi2_ump_jr_timestamp_direct(uint16_t senderClockTimestamp)
{
  return (CMIDI2_UTILITY_STATUS_JR_TIMESTAMP << 16) + senderClockTimestamp;
}

LIBREMIDI_STATIC uint32_t cmidi2_ump_jr_timestamp(double senderClockTimestamp)
{
  uint16_t value = (uint16_t)(senderClockTimestamp * JR_TIMESTAMP_TICKS_PER_SECOND);
  return (CMIDI2_UTILITY_STATUS_JR_TIMESTAMP << 16) + value;
}

LIBREMIDI_STATIC uint32_t cmidi2_ump_dctpq(uint32_t dctpq)
{
  return (CMIDI2_UTILITY_STATUS_DCTPQ << 16) + dctpq;
}

LIBREMIDI_STATIC uint32_t cmidi2_ump_dcs(uint32_t ticks)
{
  // Note that unlike JR timestamp delta clockstamps accepts ticks up to 20 bits.
  return (CMIDI2_UTILITY_STATUS_DELTA_CLOCKSTAMP << 16) + (ticks & 0xFFFFF);
}

// 7.6 System Common and System Real Time Messages
LIBREMIDI_STATIC int32_t
cmidi2_ump_system_message(uint8_t group, uint8_t status, uint8_t midi1Byte2, uint8_t midi1Byte3)
{
  return (CMIDI2_MESSAGE_TYPE_SYSTEM << 28) + ((group & 0xF) << 24) + (status << 16)
         + ((midi1Byte2 & 0x7F) << 8) + (midi1Byte3 & 0x7F);
}

// 7.3 MIDI 1.0 Channel Voice Messages
LIBREMIDI_STATIC int32_t cmidi2_ump_midi1_message(
    uint8_t group, uint8_t code, uint8_t channel, uint8_t byte3, uint8_t byte4)
{
  return (CMIDI2_MESSAGE_TYPE_MIDI_1_CHANNEL << 28) + ((group & 0xF) << 24)
         + (((code & 0xF0) + (channel & 0xF)) << 16) + ((byte3 & 0x7F) << 8) + (byte4 & 0x7F);
}

LIBREMIDI_STATIC int32_t
cmidi2_ump_midi1_note_off(uint8_t group, uint8_t channel, uint8_t note, uint8_t velocity)
{
  return cmidi2_ump_midi1_message(
      group, CMIDI2_STATUS_NOTE_OFF, channel, note & 0x7F, velocity & 0x7F);
}

LIBREMIDI_STATIC int32_t
cmidi2_ump_midi1_note_on(uint8_t group, uint8_t channel, uint8_t note, uint8_t velocity)
{
  return cmidi2_ump_midi1_message(
      group, CMIDI2_STATUS_NOTE_ON, channel, note & 0x7F, velocity & 0x7F);
}

LIBREMIDI_STATIC int32_t
cmidi2_ump_midi1_paf(uint8_t group, uint8_t channel, uint8_t note, uint8_t data)
{
  return cmidi2_ump_midi1_message(group, CMIDI2_STATUS_PAF, channel, note & 0x7F, data & 0x7F);
}

LIBREMIDI_STATIC int32_t
cmidi2_ump_midi1_cc(uint8_t group, uint8_t channel, uint8_t index, uint8_t data)
{
  return cmidi2_ump_midi1_message(group, CMIDI2_STATUS_CC, channel, index & 0x7F, data & 0x7F);
}

LIBREMIDI_STATIC int32_t cmidi2_ump_midi1_program(uint8_t group, uint8_t channel, uint8_t program)
{
  return cmidi2_ump_midi1_message(
      group, CMIDI2_STATUS_PROGRAM, channel, program & 0x7F, MIDI_2_0_RESERVED);
}

LIBREMIDI_STATIC int32_t cmidi2_ump_midi1_caf(uint8_t group, uint8_t channel, uint8_t data)
{
  return cmidi2_ump_midi1_message(
      group, CMIDI2_STATUS_CAF, channel, data & 0x7F, MIDI_2_0_RESERVED);
}

LIBREMIDI_STATIC int32_t
cmidi2_ump_midi1_pitch_bend_direct(uint8_t group, uint8_t channel, uint16_t data)
{
  return cmidi2_ump_midi1_message(
      group, CMIDI2_STATUS_PITCH_BEND, channel, data & 0x7F, (data >> 7) & 0x7F);
}

LIBREMIDI_STATIC int32_t
cmidi2_ump_midi1_pitch_bend_split(uint8_t group, uint8_t channel, uint8_t dataLSB, uint8_t dataMSB)
{
  return cmidi2_ump_midi1_message(
      group, CMIDI2_STATUS_PITCH_BEND, channel, dataLSB & 0x7F, dataMSB & 0x7F);
}

LIBREMIDI_STATIC int32_t cmidi2_ump_midi1_pitch_bend(uint8_t group, uint8_t channel, int16_t data)
{
  data += 8192;
  return cmidi2_ump_midi1_message(
      group, CMIDI2_STATUS_PITCH_BEND, channel, data & 0x7F, (data >> 7) & 0x7F);
}

// 7.4 MIDI 2.0 Channel Voice Messages
LIBREMIDI_STATIC int64_t cmidi2_ump_midi2_channel_message_8_8_16_16(
    uint8_t group, uint8_t code, uint8_t channel, uint8_t byte3, uint8_t byte4, uint16_t short1,
    uint16_t short2)
{
  return (((uint64_t)(CMIDI2_MESSAGE_TYPE_MIDI_2_CHANNEL << 28) + ((group & 0xF) << 24)
           + (((code & 0xF0) + (channel & 0xF)) << 16) + (byte3 << 8) + byte4)
          << 32)
         + ((uint64_t)short1 << 16) + short2;
}

LIBREMIDI_STATIC int64_t cmidi2_ump_midi2_channel_message_8_8_32(
    uint8_t group, uint8_t code, uint8_t channel, uint8_t byte3, uint8_t byte4, uint32_t rest)
{
  return (((uint64_t)(CMIDI2_MESSAGE_TYPE_MIDI_2_CHANNEL << 28) + ((group & 0xF) << 24)
           + (((code & 0xF0) + (channel & 0xF)) << 16) + (byte3 << 8) + byte4)
          << 32)
         + rest;
}

LIBREMIDI_STATIC uint16_t cmidi2_ump_pitch_7_9(double semitone)
{
  double actual = semitone < 0.0 ? 0.0 : semitone >= 128.0 ? 128.0 : semitone;
  uint16_t dec = (uint16_t)actual;
  double microtone = actual - dec;
  return (dec << 9) + (int)(microtone * 512.0);
}

LIBREMIDI_STATIC uint16_t cmidi2_ump_pitch_7_9_split(uint8_t semitone, double microtone)
{
  uint16_t ret = (uint16_t)(semitone & 0x7F) << 9;
  double actual = microtone < 0.0 ? 0.0 : microtone > 1.0 ? 1.0 : microtone;
  ret += (int)(actual * 512.0);
  return ret;
}

LIBREMIDI_STATIC int64_t cmidi2_ump_midi2_note_off(
    uint8_t group, uint8_t channel, uint8_t note, uint8_t attributeType, uint16_t velocity,
    uint16_t attributeData)
{
  return cmidi2_ump_midi2_channel_message_8_8_16_16(
      group, CMIDI2_STATUS_NOTE_OFF, channel, note & 0x7F, attributeType, velocity, attributeData);
}

LIBREMIDI_STATIC int64_t cmidi2_ump_midi2_note_on(
    uint8_t group, uint8_t channel, uint8_t note, uint8_t attributeType, uint16_t velocity,
    uint16_t attributeData)
{
  return cmidi2_ump_midi2_channel_message_8_8_16_16(
      group, CMIDI2_STATUS_NOTE_ON, channel, note & 0x7F, attributeType, velocity, attributeData);
}

LIBREMIDI_STATIC int64_t
cmidi2_ump_midi2_paf(uint8_t group, uint8_t channel, uint8_t note, uint32_t data)
{
  return cmidi2_ump_midi2_channel_message_8_8_32(
      group, CMIDI2_STATUS_PAF, channel, note & 0x7F, MIDI_2_0_RESERVED, data);
}

LIBREMIDI_STATIC int64_t cmidi2_ump_midi2_per_note_rcc(
    uint8_t group, uint8_t channel, uint8_t note, uint8_t index, uint32_t data)
{
  return cmidi2_ump_midi2_channel_message_8_8_32(
      group, CMIDI2_STATUS_PER_NOTE_RCC, channel, note & 0x7F, index, data);
}

LIBREMIDI_STATIC int64_t cmidi2_ump_midi2_per_note_acc(
    uint8_t group, uint8_t channel, uint8_t note, uint8_t index, uint32_t data)
{
  return cmidi2_ump_midi2_channel_message_8_8_32(
      group, CMIDI2_STATUS_PER_NOTE_ACC, channel, note & 0x7F, index, data);
}

LIBREMIDI_STATIC int64_t cmidi2_ump_midi2_per_note_management(
    uint8_t group, uint8_t channel, uint8_t note, uint8_t optionFlags)
{
  return cmidi2_ump_midi2_channel_message_8_8_32(
      group, CMIDI2_STATUS_PER_NOTE_MANAGEMENT, channel, note & 0x7F, optionFlags & 3, 0);
}

LIBREMIDI_STATIC int64_t
cmidi2_ump_midi2_cc(uint8_t group, uint8_t channel, uint8_t index, uint32_t data)
{
  return cmidi2_ump_midi2_channel_message_8_8_32(
      group, CMIDI2_STATUS_CC, channel, index & 0x7F, MIDI_2_0_RESERVED, data);
}

LIBREMIDI_STATIC int64_t cmidi2_ump_midi2_rpn(
    uint8_t group, uint8_t channel, uint8_t bankAkaMSB, uint8_t indexAkaLSB, uint32_t dataAkaDTE)
{
  return cmidi2_ump_midi2_channel_message_8_8_32(
      group, CMIDI2_STATUS_RPN, channel, bankAkaMSB & 0x7F, indexAkaLSB & 0x7F, dataAkaDTE);
}

LIBREMIDI_STATIC int64_t cmidi2_ump_midi2_nrpn(
    uint8_t group, uint8_t channel, uint8_t bankAkaMSB, uint8_t indexAkaLSB, uint32_t dataAkaDTE)
{
  return cmidi2_ump_midi2_channel_message_8_8_32(
      group, CMIDI2_STATUS_NRPN, channel, bankAkaMSB & 0x7F, indexAkaLSB & 0x7F, dataAkaDTE);
}

LIBREMIDI_STATIC int64_t cmidi2_ump_midi2_relative_rpn(
    uint8_t group, uint8_t channel, uint8_t bankAkaMSB, uint8_t indexAkaLSB, uint32_t dataAkaDTE)
{
  return cmidi2_ump_midi2_channel_message_8_8_32(
      group, CMIDI2_STATUS_RELATIVE_RPN, channel, bankAkaMSB & 0x7F, indexAkaLSB & 0x7F,
      dataAkaDTE);
}

LIBREMIDI_STATIC int64_t cmidi2_ump_midi2_relative_nrpn(
    uint8_t group, uint8_t channel, uint8_t bankAkaMSB, uint8_t indexAkaLSB, uint32_t dataAkaDTE)
{
  return cmidi2_ump_midi2_channel_message_8_8_32(
      group, CMIDI2_STATUS_RELATIVE_NRPN, channel, bankAkaMSB & 0x7F, indexAkaLSB & 0x7F,
      dataAkaDTE);
}

LIBREMIDI_STATIC int64_t cmidi2_ump_midi2_program(
    uint8_t group, uint8_t channel, uint8_t optionFlags, uint8_t program, uint8_t bankMSB,
    uint8_t bankLSB)
{
  return cmidi2_ump_midi2_channel_message_8_8_32(
      group, CMIDI2_STATUS_PROGRAM, channel, MIDI_2_0_RESERVED, optionFlags & 1,
      ((program & 0x7F) << 24) + (bankMSB << 8) + bankLSB);
}

LIBREMIDI_STATIC int64_t cmidi2_ump_midi2_caf(uint8_t group, uint8_t channel, uint32_t data)
{
  return cmidi2_ump_midi2_channel_message_8_8_32(
      group, CMIDI2_STATUS_CAF, channel, MIDI_2_0_RESERVED, MIDI_2_0_RESERVED, data);
}

LIBREMIDI_STATIC int64_t
cmidi2_ump_midi2_pitch_bend_direct(uint8_t group, uint8_t channel, uint32_t unsignedData)
{
  return cmidi2_ump_midi2_channel_message_8_8_32(
      group, CMIDI2_STATUS_PITCH_BEND, channel, MIDI_2_0_RESERVED, MIDI_2_0_RESERVED,
      unsignedData);
}

LIBREMIDI_STATIC int64_t cmidi2_ump_midi2_pitch_bend(uint8_t group, uint8_t channel, int32_t data)
{
  return cmidi2_ump_midi2_pitch_bend_direct(group, channel, 0x80000000 + data);
}

LIBREMIDI_STATIC int64_t cmidi2_ump_midi2_per_note_pitch_bend_direct(
    uint8_t group, uint8_t channel, uint8_t note, uint32_t data)
{
  return cmidi2_ump_midi2_channel_message_8_8_32(
      group, CMIDI2_STATUS_PER_NOTE_PITCH_BEND, channel, note & 0x7F, MIDI_2_0_RESERVED, data);
}

LIBREMIDI_STATIC int64_t
cmidi2_ump_midi2_per_note_pitch_bend(uint8_t group, uint8_t channel, uint8_t note, uint32_t data)
{
  return cmidi2_ump_midi2_per_note_pitch_bend_direct(group, channel, note, 0x80000000 + data);
}

// Common utility functions for sysex support

LIBREMIDI_STATIC uint8_t cmidi2_ump_get_byte_from_uint32(uint32_t src, uint8_t index)
{
  return (uint8_t)(src >> ((7 - index) * 8) & 0xFF);
}

LIBREMIDI_STATIC uint8_t cmidi2_ump_get_byte_from_uint64(uint64_t src, uint8_t index)
{
  return (uint8_t)(src >> ((7 - index) * 8) & 0xFF);
}

LIBREMIDI_STATIC size_t cmidi2_ump_sysex_get_num_packets(size_t numBytes, uint8_t radix)
{
  return numBytes <= radix ? 1 : (numBytes / radix + (numBytes % radix ? 1 : 0));
}

LIBREMIDI_STATIC uint32_t cmidi2_ump_read_uint32_bytes_le(const void* sequence)
{
  const uint8_t* bytes = (const uint8_t*)sequence;
  uint32_t ret = 0;
  for (int i = 0; i < 4; i++)
    ret += ((uint32_t)bytes[i]) << (i * 8);
  return ret;
}

LIBREMIDI_STATIC uint32_t cmidi2_ump_read_uint32_bytes_be(const void* sequence)
{
  const uint8_t* bytes = (const uint8_t*)sequence;
  uint32_t ret = 0;
  for (int i = 0; i < 4; i++)
    ret += ((uint32_t)bytes[i]) << ((3 - i) * 8);
  return ret;
}

LIBREMIDI_STATIC bool cmidi2_util_is_platform_little_endian()
{
  int i = 1;
  return *(char*)&i;
}

LIBREMIDI_STATIC uint32_t cmidi2_ump_read_uint32_bytes(const void* sequence)
{
  return cmidi2_util_is_platform_little_endian() ? cmidi2_ump_read_uint32_bytes_le(sequence)
                                                 : cmidi2_ump_read_uint32_bytes_be(sequence);
}

LIBREMIDI_STATIC uint64_t cmidi2_ump_read_uint64_bytes_le(const void* sequence)
{
  return ((uint64_t)cmidi2_ump_read_uint32_bytes_le(sequence) << 32)
         + cmidi2_ump_read_uint32_bytes_le((const uint8_t*)sequence + 4);
}

LIBREMIDI_STATIC uint64_t cmidi2_ump_read_uint64_bytes_be(const void* sequence)
{
  return ((uint64_t)cmidi2_ump_read_uint32_bytes_be(sequence) << 32)
         + cmidi2_ump_read_uint32_bytes_be((const uint8_t*)sequence + 4);
}

LIBREMIDI_STATIC uint64_t cmidi2_ump_read_uint64_bytes(const void* sequence)
{
  return cmidi2_util_is_platform_little_endian() ? cmidi2_ump_read_uint64_bytes_le(sequence)
                                                 : cmidi2_ump_read_uint64_bytes_be(sequence);
}

LIBREMIDI_STATIC void cmidi2_ump_sysex_get_packet_of(
    uint64_t* result1, uint64_t* result2, uint8_t group, size_t numBytes, const void* srcData,
    int32_t index, enum cmidi2_message_type messageType, int radix, bool hasStreamId,
    uint8_t streamId)
{
  uint8_t dst8[16];
  memset(dst8, 0, 16);
  const uint8_t* src8 = (const uint8_t*)srcData;

  dst8[0] = (messageType << 4) + (group & 0xF);

  enum cmidi2_sysex_status status;
  uint8_t size;
  if (numBytes <= (size_t)radix)
  {
    status = CMIDI2_SYSEX_IN_ONE_UMP;
    size = numBytes; // single packet message
  }
  else if (index == 0)
  {
    status = CMIDI2_SYSEX_START;
    size = radix;
  }
  else
  {
    uint8_t isEnd = (size_t)index == cmidi2_ump_sysex_get_num_packets(numBytes, radix) - 1;
    if (isEnd)
    {
      size = numBytes % radix ? numBytes % radix : radix;
      status = CMIDI2_SYSEX_END;
    }
    else
    {
      size = radix;
      status = CMIDI2_SYSEX_CONTINUE;
    }
  }
  dst8[1] = status + size + (hasStreamId ? 1 : 0);

  if (hasStreamId)
    dst8[2] = streamId;

  uint8_t dstOffset = hasStreamId ? 3 : 2;
  for (size_t i = 0, j = index * radix; i < size; i++, j++)
    dst8[i + dstOffset] = src8[j];

  *result1 = cmidi2_ump_read_uint64_bytes_be(dst8);
  if (result2)
    *result2 = cmidi2_ump_read_uint64_bytes_be(dst8 + 8);
}

// 7.7 System Exclusive 7-Bit Messages
LIBREMIDI_STATIC uint64_t cmidi2_ump_sysex7_direct(
    uint8_t group, uint8_t status, uint8_t numBytes, uint8_t data1, uint8_t data2, uint8_t data3,
    uint8_t data4, uint8_t data5, uint8_t data6)
{
  return (((uint64_t)((CMIDI2_MESSAGE_TYPE_SYSEX7 << 28) + ((group & 0xF) << 24)
                      + ((status + numBytes) << 16)))
          << 32)
         + ((uint64_t)data1 << 40) + ((uint64_t)data2 << 32) + (data3 << 24) + (data4 << 16)
         + (data5 << 8) + data6;
}

LIBREMIDI_STATIC uint32_t cmidi2_ump_sysex7_get_sysex_length(const void* srcData)
{
  int i = 0;
  const uint8_t* csrc = (const uint8_t*)srcData;
  while (csrc[i] != 0xF7)
    i++;
  /* This function automatically detects if 0xF0 is prepended and reduce length if it is. */
  return i - (csrc[0] == 0xF0 ? 1 : 0);
}

LIBREMIDI_STATIC size_t cmidi2_ump_sysex7_get_num_packets(size_t numSysex7Bytes)
{
  return cmidi2_ump_sysex_get_num_packets(numSysex7Bytes, 6);
}

LIBREMIDI_STATIC uint64_t
cmidi2_ump_sysex7_get_packet_of(uint8_t group, size_t numBytes, const void* srcData, int32_t index)
{
  uint64_t result;
  int srcOffset = numBytes > 0 && ((const uint8_t*)srcData)[0] == 0xF0 ? 1 : 0;
  cmidi2_ump_sysex_get_packet_of(
      &result, NULL, group, numBytes, (const uint8_t*)srcData + srcOffset, index,
      CMIDI2_MESSAGE_TYPE_SYSEX7, 6, false, 0);
  return result;
}

/* process() - more complicated function */

// This returns NULL for success, or anything else for failure.
typedef void* (*cmidi2_ump_handler_u64)(uint64_t data, void* context);

// Processes sysex7 inputs where we do not always end at F7 and thus takes length as the argument.
// This returns NULL for success, or anything else that `sendUMP` returns for failure.
LIBREMIDI_STATIC void* cmidi2_ump_sysex7_process_n(
    uint8_t group, void* sysex, uint32_t length, cmidi2_ump_handler_u64 sendUMP, void* context)
{
  int32_t numPackets = cmidi2_ump_sysex7_get_num_packets(length);
  for (int p = 0; p < numPackets; p++)
  {
    int64_t ump = cmidi2_ump_sysex7_get_packet_of(group, length, sysex, p);
    void* retCode = sendUMP(ump, context);
    if (retCode != 0)
      return retCode;
  }
  return NULL;
}

// This returns NULL for success, or anything else that `sendUMP` returns for failure.
LIBREMIDI_STATIC void* cmidi2_ump_sysex7_process(
    uint8_t group, void* sysex, cmidi2_ump_handler_u64 sendUMP, void* context)
{
  uint32_t length = cmidi2_ump_sysex7_get_sysex_length(sysex);
  return cmidi2_ump_sysex7_process_n(group, sysex, length, sendUMP, context);
}

// 7.8 System Exclusive 8-Bit Messages

LIBREMIDI_STATIC size_t cmidi2_ump_sysex8_get_num_packets(size_t numBytes)
{
  return cmidi2_ump_sysex_get_num_packets(numBytes, 13);
}

LIBREMIDI_STATIC void cmidi2_ump_sysex8_get_packet_of(
    uint8_t group, uint8_t streamId, size_t numBytes, const void* srcData, size_t index,
    uint64_t* result1, uint64_t* result2)
{
  cmidi2_ump_sysex_get_packet_of(
      result1, result2, group, numBytes, srcData, index, CMIDI2_MESSAGE_TYPE_SYSEX8_MDS, 13, true,
      streamId);
}

/* process() - more complicated function */

// This returns NULL for success, or anything else for failure.
typedef void* (*cmidi2_ump_handler_u128)(
    uint64_t data1, uint64_t data2, size_t index, void* context);

// This returns NULL for success, or anything else that `sendUMP` returns for failure.
LIBREMIDI_STATIC void* cmidi2_ump_sysex8_process(
    uint8_t group, void* sysex, uint32_t length, uint8_t streamId, cmidi2_ump_handler_u128 sendUMP,
    void* context)
{
  uint32_t numPackets = cmidi2_ump_sysex8_get_num_packets(length);
  for (size_t p = 0; p < numPackets; p++)
  {
    uint64_t result1, result2;
    cmidi2_ump_sysex8_get_packet_of(group, streamId, length, sysex, p, &result1, &result2);
    void* retCode = sendUMP(result1, result2, p, context);
    if (retCode != 0)
      return retCode;
  }
  return NULL;
}

// 7.9 Mixed Data Set Message

LIBREMIDI_STATIC uint16_t cmidi2_ump_mds_get_num_chunks(uint32_t numTotalBytesInMDS)
{
  uint32_t radix = 14 * 0x10000;
  return numTotalBytesInMDS / radix + (numTotalBytesInMDS % radix ? 1 : 0);
}

// Returns -1 if input is out of range
LIBREMIDI_STATIC int32_t cmidi2_ump_mds_get_num_payloads(uint32_t numTotalBytesinChunk)
{
  if (numTotalBytesinChunk > 14 * 65535)
    return -1;
  return numTotalBytesinChunk / 14 + (numTotalBytesinChunk % 14 ? 1 : 0);
}

LIBREMIDI_STATIC void cmidi2_ump_mds_get_header(
    uint8_t group, uint8_t mdsId, uint16_t numBytesInChunk, uint16_t numChunks,
    uint16_t chunkIndex, uint16_t manufacturerId, uint16_t deviceId, uint16_t subId,
    uint16_t subId2, uint64_t* result1, uint64_t* result2)
{
  uint8_t dst8[16];
  memset(dst8, 0, 16);

  dst8[0] = (CMIDI2_MESSAGE_TYPE_SYSEX8_MDS << 4) + (group & 0xF);
  dst8[1] = CMIDI2_MIXED_DATA_STATUS_HEADER + mdsId;
  *((uint16_t*)(void*)(dst8 + 2)) = numBytesInChunk;
  *((uint16_t*)(void*)(dst8 + 4)) = numChunks;
  *((uint16_t*)(void*)(dst8 + 6)) = chunkIndex;
  *((uint16_t*)(void*)(dst8 + 8)) = manufacturerId;
  *((uint16_t*)(void*)(dst8 + 10)) = deviceId;
  *((uint16_t*)(void*)(dst8 + 12)) = subId;
  *((uint16_t*)(void*)(dst8 + 14)) = subId2;

  *result1 = cmidi2_ump_read_uint64_bytes_be(dst8);
  if (result2)
    *result2 = cmidi2_ump_read_uint64_bytes_be(dst8 + 8);
}

// srcData points to exact start of the source data.
LIBREMIDI_STATIC void cmidi2_ump_mds_get_payload_of(
    uint8_t group, uint8_t mdsId, uint16_t numBytes, const void* srcData, uint64_t* result1,
    uint64_t* result2)
{
  uint8_t dst8[16];
  memset(dst8, 0, 16);
  const uint8_t* src8 = (const uint8_t*)srcData;

  dst8[0] = (CMIDI2_MESSAGE_TYPE_SYSEX8_MDS << 4) + (group & 0xF);
  dst8[1] = CMIDI2_MIXED_DATA_STATUS_PAYLOAD + mdsId;

  uint8_t radix = 14;
  uint8_t size = numBytes < radix ? numBytes % radix : radix;

  for (uint8_t i = 0; i < size; i++)
    dst8[i + 2] = src8[i];

  *result1 = cmidi2_ump_read_uint64_bytes_be(dst8);
  if (result2)
    *result2 = cmidi2_ump_read_uint64_bytes_be(dst8 + 8);
}

/* process() - more complicated function */
// This returns NULL for success, or anything else for failure.
typedef void* (*cmidi2_mds_handler)(
    uint64_t data1, uint64_t data2, size_t chunkId, size_t payloadId, void* context);

// This returns NULL for success, or anything else that `sendUMP` returns for failure.
LIBREMIDI_STATIC void* cmidi2_ump_mds_process(
    uint8_t group, uint8_t mdsId, void* data, uint32_t length, cmidi2_mds_handler sendUMP,
    void* context)
{
  int32_t numChunks = cmidi2_ump_mds_get_num_chunks(length);
  for (int c = 0; c < numChunks; c++)
  {
    int32_t maxChunkSize = 14 * 65535;
    int32_t chunkSize = c + 1 == numChunks ? (int32_t)(length % maxChunkSize) : maxChunkSize;
    int32_t numPayloads = cmidi2_ump_mds_get_num_payloads(chunkSize);
    for (int p = 0; p < numPayloads; p++)
    {
      uint64_t result1, result2;
      size_t offset = 14 * (65536 * c + p);
      cmidi2_ump_mds_get_payload_of(
          group, mdsId, chunkSize, (uint8_t*)data + offset, &result1, &result2);
      void* retCode = sendUMP(result1, result2, c, p, context);
      if (retCode)
        return retCode;
    }
  }
  return NULL;
}

// 7.5 Flex Data Message

LIBREMIDI_STATIC uint16_t cmidi2_ump_flex_data_get_num_packets(uint32_t numTotalBytesInFlexData)
{
  return numTotalBytesInFlexData / 12 + (numTotalBytesInFlexData % 12 ? 1 : 0);
}

LIBREMIDI_STATIC void cmidi2_ump_flex_data_complete_packet(
    uint8_t group, uint8_t addressing, uint8_t channel, uint8_t statusBank, uint8_t statusCode,
    uint32_t data1, uint32_t data2, uint32_t data3, uint64_t* result1, uint64_t* result2)
{
  uint8_t dst8[4];
  memset(dst8, 0, 4);
  dst8[0] = (CMIDI2_MESSAGE_TYPE_FLEX_DATA << 4) + (group & 0xF);
  dst8[1] = (CMIDI2_SYSEX_IN_ONE_UMP << 2) + ((addressing & 0x3) << 4) + (channel & 0xF);
  dst8[2] = statusBank;
  dst8[3] = statusCode;

  *result1 = ((uint64_t)cmidi2_ump_read_uint32_bytes_be(dst8) << 32) + data1;
  *result2 = ((uint64_t)data2 << 32) + data3;
}

LIBREMIDI_STATIC void cmidi2_ump_flex_data_get_packet_of(
    uint8_t group, uint8_t addressing, uint8_t channel, uint8_t statusBank, uint8_t statusCode,
    uint16_t numBytes, const void* srcData, int32_t currentPacket, uint64_t* result1,
    uint64_t* result2)
{
  uint8_t dst8[16];
  memset(dst8, 0, 16);
  const uint8_t* src8 = (const uint8_t*)srcData;

  dst8[0] = (CMIDI2_MESSAGE_TYPE_FLEX_DATA << 4) + (group & 0xF);

  const size_t radix = 12;
  enum cmidi2_sysex_status format;
  uint8_t size;
  if (numBytes <= radix)
  {
    format = CMIDI2_SYSEX_IN_ONE_UMP;
    size = numBytes; // single packet message
  }
  else if (currentPacket == 0)
  {
    format = CMIDI2_SYSEX_START;
    size = radix;
  }
  else
  {
    uint8_t isEnd = (size_t)currentPacket == cmidi2_ump_sysex_get_num_packets(numBytes, radix) - 1;
    if (isEnd)
    {
      size = numBytes % radix ? numBytes % radix : radix;
      format = CMIDI2_SYSEX_END;
    }
    else
    {
      size = radix;
      format = CMIDI2_SYSEX_CONTINUE;
    }
  }
  dst8[1] = (format << 2) + ((addressing & 0x3) << 4) + (channel & 0xF);
  dst8[2] = statusBank;
  dst8[3] = statusCode;

  for (uint8_t i = 0; i < size; i++)
    dst8[i + 4] = src8[i];

  *result1 = cmidi2_ump_read_uint64_bytes_be(dst8);
  *result2 = cmidi2_ump_read_uint64_bytes_be(dst8 + 8);
}

/* process() - more complicated function */
// This returns NULL for success, or anything else for failure.
typedef void* (*cmidi2_flex_data_handler)(uint64_t data1, uint64_t data2, void* context);

// This returns NULL for success, or anything else that `sendUMP` returns for failure.
// `text` is usually a null-terminated text string, but it may contain `\0` as Melisma in lyricText. Hence we still need `length`.
LIBREMIDI_STATIC void* cmidi2_ump_flex_data_process(
    uint8_t group, uint8_t addressing, uint8_t channel, uint8_t statusBank, uint8_t statusCode,
    const char* text, uint32_t length, cmidi2_flex_data_handler sendUMP, void* context)
{
  int32_t numPackets = cmidi2_ump_flex_data_get_num_packets(length);
  for (int p = 0; p < numPackets; p++)
  {
    uint64_t result1, result2;
    cmidi2_ump_flex_data_get_packet_of(
        group, addressing, channel, statusBank, statusCode, length, text, p, &result1, &result2);
    void* retCode = sendUMP(result1, result2, context);
    if (retCode != 0)
      return retCode;
  }
  return NULL;
}

// individual flex data message generators

LIBREMIDI_STATIC void cmidi2_ump_flex_data_set_tempo_direct(
    uint8_t group, uint8_t channel, uint32_t tempoIn10NanosecondsPerQN, uint64_t* result1,
    uint64_t* result2)
{
  cmidi2_ump_flex_data_complete_packet(
      group, 1, channel, CMIDI2_FLEX_DATA_BANK_SETUP_AND_PERFORMANCE,
      CMIDI2_FLEX_DATA_STATUS_SET_TEMPO, tempoIn10NanosecondsPerQN, 0, 0, result1, result2);
}

LIBREMIDI_STATIC void cmidi2_ump_flex_data_set_time_signature(
    uint8_t group, uint8_t channel, uint8_t numerator, uint8_t denominator,
    uint8_t numberOf32thNotes, uint64_t* result1, uint64_t* result2)
{
  cmidi2_ump_flex_data_complete_packet(
      group, 1, channel, CMIDI2_FLEX_DATA_BANK_SETUP_AND_PERFORMANCE,
      CMIDI2_FLEX_DATA_STATUS_SET_TIME_SIGNATURE,
      (numerator << 24) + (denominator << 16) + (numberOf32thNotes << 8), 0, 0, result1, result2);
}

LIBREMIDI_STATIC void cmidi2_ump_flex_data_set_metronome(
    uint8_t group, uint8_t channel, uint8_t clocksPerPrimaryClick, uint8_t barAccent1,
    uint8_t barAccent2, uint8_t barAccent3, uint8_t subDivisionClicks1, uint8_t subDivisionClicks2,
    uint64_t* result1, uint64_t* result2)
{
  cmidi2_ump_flex_data_complete_packet(
      group, 1, channel, CMIDI2_FLEX_DATA_BANK_SETUP_AND_PERFORMANCE,
      CMIDI2_FLEX_DATA_STATUS_SET_METRONOME,
      (clocksPerPrimaryClick << 24) + (barAccent1 << 16) + (barAccent2 << 8) + barAccent3,
      (subDivisionClicks1 << 24) + (subDivisionClicks2 << 16), 0, result1, result2);
}

LIBREMIDI_STATIC void cmidi2_ump_flex_data_set_key_signature(
    uint8_t group, uint8_t addressing, uint8_t channel, uint8_t sharpsFlats, uint8_t tonicNote,
    uint64_t* result1, uint64_t* result2)
{
  cmidi2_ump_flex_data_complete_packet(
      group, addressing, channel, CMIDI2_FLEX_DATA_BANK_SETUP_AND_PERFORMANCE,
      CMIDI2_FLEX_DATA_STATUS_SET_KEY_SIGNATURE, (sharpsFlats << 24) + (tonicNote << 16), 0, 0,
      result1, result2);
}

LIBREMIDI_STATIC void cmidi2_ump_flex_data_set_chord_name(
    uint8_t group, uint8_t addressing, uint8_t channel, uint8_t sharpsFlats, uint8_t chordTonic,
    uint8_t chordType, uint8_t alter1Type, uint8_t alter1Degree, uint8_t alter2Type,
    uint8_t alter2Degree, uint8_t alter3Type, uint8_t alter3Degree, uint8_t alter4Type,
    uint8_t alter4Degree, uint8_t bassSharpsFlats, uint8_t bassNote, uint8_t bassChordType,
    uint8_t bassAlter1Type, uint8_t bassAlter1Degree, uint8_t bassAlter2Type,
    uint8_t bassAlter2Degree, uint64_t* result1, uint64_t* result2)
{
  cmidi2_ump_flex_data_complete_packet(
      group, addressing, channel, CMIDI2_FLEX_DATA_BANK_SETUP_AND_PERFORMANCE,
      CMIDI2_FLEX_DATA_STATUS_SET_CHORD_NAME,
      (sharpsFlats << 28) + (chordTonic << 24) + (chordType << 16) + (alter1Type << 12)
          + (alter1Degree << 8) + (alter2Type << 4) + alter2Degree,
      (alter3Type << 28) + (alter3Degree << 24) + (alter4Type << 20) + (alter4Degree << 16),
      (bassSharpsFlats << 28) + (bassNote << 24) + (bassChordType << 16) + (bassAlter1Type << 12)
          + (bassAlter1Degree << 8) + (bassAlter2Type << 4) + bassAlter2Degree,
      result1, result2);
}

// --------
// Strongly-typed(?) UMP.
// I kind of think those getters are overkill, so I would collect almost use `cmidi2_ump_get_xxx()`
// as those strongly typed functions, so that those who don't want them can safely ignore them.

typedef uint32_t cmidi2_ump;

LIBREMIDI_STATIC void cmidi2_ump_write32(cmidi2_ump* dst, uint32_t value)
{
  dst[0] = value;
}

LIBREMIDI_STATIC void cmidi2_ump_write64(cmidi2_ump* dst, uint64_t value)
{
  dst[0] = value >> 32;
  dst[1] = value & 0xFFFFFFFF;
}

LIBREMIDI_STATIC void cmidi2_ump_write128(cmidi2_ump* dst, uint64_t value1, uint64_t value2)
{
  dst[0] = value1 >> 32;
  dst[1] = value1 & 0xFFFFFFFF;
  dst[2] = value2 >> 32;
  dst[3] = value2 & 0xFFFFFFFF;
}

LIBREMIDI_STATIC uint8_t cmidi2_ump_get_byte_at(const cmidi2_ump* ump, uint8_t at)
{
  ump += at / 4;
  switch (at % 4)
  {
    case 0:
      return (*ump & 0xFF000000) >> 24;
    case 1:
      return (*ump & 0xFF0000) >> 16;
    case 2:
      return (*ump & 0xFF00) >> 8;
    case 3:
      return *ump & 0xFF;
  }
  return 0; // This is unexpected.
}

LIBREMIDI_STATIC uint8_t cmidi2_ump_get_message_type(const cmidi2_ump* ump)
{
  return *ump >> 28;
}

LIBREMIDI_STATIC uint8_t cmidi2_ump_get_message_size_bytes(const cmidi2_ump* ump)
{
  switch (cmidi2_ump_get_message_type(ump))
  {
    case CMIDI2_MESSAGE_TYPE_UTILITY:
    case CMIDI2_MESSAGE_TYPE_SYSTEM:
    case CMIDI2_MESSAGE_TYPE_MIDI_1_CHANNEL:
      return 4;
    case CMIDI2_MESSAGE_TYPE_SYSEX7:
    case CMIDI2_MESSAGE_TYPE_MIDI_2_CHANNEL:
      return 8;
    case CMIDI2_MESSAGE_TYPE_SYSEX8_MDS:
      return 16;
  }
  return 0; // invalid
}

LIBREMIDI_STATIC uint8_t cmidi2_ump_get_group(const cmidi2_ump* ump)
{
  return (*ump >> 24) & 0xF;
}

LIBREMIDI_STATIC uint8_t cmidi2_ump_get_status_byte(const cmidi2_ump* ump)
{
  return (*ump >> 16) & 0xFF;
}

LIBREMIDI_STATIC uint8_t cmidi2_ump_get_status_code(const cmidi2_ump* ump)
{
  return (*ump >> 16) & 0xF0;
}

LIBREMIDI_STATIC uint8_t cmidi2_ump_get_channel(const cmidi2_ump* ump)
{
  return (*ump >> 16) & 0xF;
}

LIBREMIDI_STATIC uint32_t cmidi2_ump_get_32_to_64(const cmidi2_ump* ump)
{
  return *(ump + 1);
}

LIBREMIDI_STATIC uint16_t cmidi2_ump_get_jr_clock_time(const cmidi2_ump* ump)
{
  return (cmidi2_ump_get_byte_at(ump, 2) << 8) + cmidi2_ump_get_byte_at(ump, 3);
}
LIBREMIDI_STATIC uint16_t cmidi2_ump_get_jr_timestamp_timestamp(const cmidi2_ump* ump)
{
  return (cmidi2_ump_get_byte_at(ump, 2) << 8) + cmidi2_ump_get_byte_at(ump, 3);
}

LIBREMIDI_STATIC uint16_t cmidi2_ump_get_dctpq(const cmidi2_ump* ump)
{
  return (cmidi2_ump_get_byte_at(ump, 2) << 8) + cmidi2_ump_get_byte_at(ump, 3);
}

LIBREMIDI_STATIC uint16_t cmidi2_ump_get_dcs(const cmidi2_ump* ump)
{
  return ((cmidi2_ump_get_byte_at(ump, 1) & 0xF) << 16) + (cmidi2_ump_get_byte_at(ump, 2) << 8)
         + cmidi2_ump_get_byte_at(ump, 3);
}

LIBREMIDI_STATIC uint8_t cmidi2_ump_get_system_message_byte2(const cmidi2_ump* ump)
{
  return cmidi2_ump_get_byte_at(ump, 2);
}
LIBREMIDI_STATIC uint8_t cmidi2_ump_get_system_message_byte3(const cmidi2_ump* ump)
{
  return cmidi2_ump_get_byte_at(ump, 3);
}

LIBREMIDI_STATIC uint8_t cmidi2_ump_get_midi1_byte2(const cmidi2_ump* ump)
{
  return cmidi2_ump_get_byte_at(ump, 2);
}
LIBREMIDI_STATIC uint8_t cmidi2_ump_get_midi1_byte3(const cmidi2_ump* ump)
{
  return cmidi2_ump_get_byte_at(ump, 3);
}

LIBREMIDI_STATIC uint8_t cmidi2_ump_get_midi1_note_note(const cmidi2_ump* ump)
{
  return cmidi2_ump_get_byte_at(ump, 2);
}
LIBREMIDI_STATIC uint8_t cmidi2_ump_get_midi1_note_velocity(const cmidi2_ump* ump)
{
  return cmidi2_ump_get_byte_at(ump, 3);
}
LIBREMIDI_STATIC uint8_t cmidi2_ump_get_midi1_paf_note(const cmidi2_ump* ump)
{
  return cmidi2_ump_get_byte_at(ump, 2);
}
LIBREMIDI_STATIC uint8_t cmidi2_ump_get_midi1_paf_data(const cmidi2_ump* ump)
{
  return cmidi2_ump_get_byte_at(ump, 3);
}
LIBREMIDI_STATIC uint8_t cmidi2_ump_get_midi1_cc_index(const cmidi2_ump* ump)
{
  return cmidi2_ump_get_byte_at(ump, 2);
}
LIBREMIDI_STATIC uint8_t cmidi2_ump_get_midi1_cc_data(const cmidi2_ump* ump)
{
  return cmidi2_ump_get_byte_at(ump, 3);
}
LIBREMIDI_STATIC uint8_t cmidi2_ump_get_midi1_program_program(const cmidi2_ump* ump)
{
  return cmidi2_ump_get_byte_at(ump, 2);
}
LIBREMIDI_STATIC uint8_t cmidi2_ump_get_midi1_caf_data(const cmidi2_ump* ump)
{
  return cmidi2_ump_get_byte_at(ump, 2);
}
LIBREMIDI_STATIC uint16_t cmidi2_ump_get_midi1_pitch_bend_data(const cmidi2_ump* ump)
{
  return cmidi2_ump_get_byte_at(ump, 2) + cmidi2_ump_get_byte_at(ump, 3) * 0x80;
}

LIBREMIDI_STATIC uint8_t cmidi2_ump_get_sysex7_num_bytes(const cmidi2_ump* ump)
{
  return cmidi2_ump_get_channel(ump); // same bits
}

LIBREMIDI_STATIC uint8_t cmidi2_ump_get_midi2_note_note(const cmidi2_ump* ump)
{
  return cmidi2_ump_get_byte_at(ump, 2);
}
LIBREMIDI_STATIC uint8_t cmidi2_ump_get_midi2_note_attribute_type(const cmidi2_ump* ump)
{
  return cmidi2_ump_get_byte_at(ump, 3);
}
LIBREMIDI_STATIC uint16_t cmidi2_ump_get_midi2_note_velocity(const cmidi2_ump* ump)
{
  return (cmidi2_ump_get_byte_at(ump, 4) << 8) + (cmidi2_ump_get_byte_at(ump, 5));
}
LIBREMIDI_STATIC uint16_t cmidi2_ump_get_midi2_note_attribute_data(const cmidi2_ump* ump)
{
  return (cmidi2_ump_get_byte_at(ump, 6) << 8) + (cmidi2_ump_get_byte_at(ump, 7));
}
LIBREMIDI_STATIC uint8_t cmidi2_ump_get_midi2_paf_note(const cmidi2_ump* ump)
{
  return cmidi2_ump_get_byte_at(ump, 2);
}
LIBREMIDI_STATIC uint32_t cmidi2_ump_get_midi2_paf_data(const cmidi2_ump* ump)
{
  return cmidi2_ump_get_32_to_64(ump);
}
LIBREMIDI_STATIC uint8_t cmidi2_ump_get_midi2_pnrcc_note(const cmidi2_ump* ump)
{
  return cmidi2_ump_get_byte_at(ump, 2);
}
LIBREMIDI_STATIC uint8_t cmidi2_ump_get_midi2_pnrcc_index(const cmidi2_ump* ump)
{
  return cmidi2_ump_get_byte_at(ump, 3);
}
LIBREMIDI_STATIC uint32_t cmidi2_ump_get_midi2_pnrcc_data(const cmidi2_ump* ump)
{
  return cmidi2_ump_get_32_to_64(ump);
}
LIBREMIDI_STATIC uint8_t cmidi2_ump_get_midi2_pnacc_note(const cmidi2_ump* ump)
{
  return cmidi2_ump_get_byte_at(ump, 2);
}
LIBREMIDI_STATIC uint8_t cmidi2_ump_get_midi2_pnacc_index(const cmidi2_ump* ump)
{
  return cmidi2_ump_get_byte_at(ump, 3);
}
LIBREMIDI_STATIC uint32_t cmidi2_ump_get_midi2_pnacc_data(const cmidi2_ump* ump)
{
  return cmidi2_ump_get_32_to_64(ump);
}
LIBREMIDI_STATIC uint8_t cmidi2_ump_get_midi2_pn_management_note(const cmidi2_ump* ump)
{
  return cmidi2_ump_get_byte_at(ump, 2);
}
LIBREMIDI_STATIC uint32_t cmidi2_ump_get_midi2_pn_management_options(const cmidi2_ump* ump)
{
  return cmidi2_ump_get_byte_at(ump, 3);
}
LIBREMIDI_STATIC uint8_t cmidi2_ump_get_midi2_cc_index(const cmidi2_ump* ump)
{
  return cmidi2_ump_get_byte_at(ump, 2);
}
LIBREMIDI_STATIC uint32_t cmidi2_ump_get_midi2_cc_data(const cmidi2_ump* ump)
{
  return cmidi2_ump_get_32_to_64(ump);
}
// absolute RPN or relative RPN
LIBREMIDI_STATIC uint8_t cmidi2_ump_get_midi2_rpn_msb(const cmidi2_ump* ump)
{
  return cmidi2_ump_get_byte_at(ump, 2);
}
// absolute RPN or relative RPN
LIBREMIDI_STATIC uint8_t cmidi2_ump_get_midi2_rpn_lsb(const cmidi2_ump* ump)
{
  return cmidi2_ump_get_byte_at(ump, 3);
}
// absolute RPN or relative RPN
LIBREMIDI_STATIC uint32_t cmidi2_ump_get_midi2_rpn_data(const cmidi2_ump* ump)
{
  return cmidi2_ump_get_32_to_64(ump);
}
// absolute NRPN or relative NRPN
LIBREMIDI_STATIC uint8_t cmidi2_ump_get_midi2_nrpn_msb(const cmidi2_ump* ump)
{
  return cmidi2_ump_get_byte_at(ump, 2);
}
// absolute NRPN or relative NRPN
LIBREMIDI_STATIC uint8_t cmidi2_ump_get_midi2_nrpn_lsb(const cmidi2_ump* ump)
{
  return cmidi2_ump_get_byte_at(ump, 3);
}
// absolute NRPN or relative NRPN
LIBREMIDI_STATIC uint32_t cmidi2_ump_get_midi2_nrpn_data(const cmidi2_ump* ump)
{
  return cmidi2_ump_get_32_to_64(ump);
}
LIBREMIDI_STATIC uint8_t cmidi2_ump_get_midi2_program_options(const cmidi2_ump* ump)
{
  return cmidi2_ump_get_byte_at(ump, 3);
}
LIBREMIDI_STATIC uint8_t cmidi2_ump_get_midi2_program_program(const cmidi2_ump* ump)
{
  return cmidi2_ump_get_byte_at(ump, 4);
}
LIBREMIDI_STATIC uint8_t cmidi2_ump_get_midi2_program_bank_msb(const cmidi2_ump* ump)
{
  return (cmidi2_ump_get_byte_at(ump, 6));
}
LIBREMIDI_STATIC uint8_t cmidi2_ump_get_midi2_program_bank_lsb(const cmidi2_ump* ump)
{
  return (cmidi2_ump_get_byte_at(ump, 7));
}
LIBREMIDI_STATIC uint32_t cmidi2_ump_get_midi2_caf_data(const cmidi2_ump* ump)
{
  return cmidi2_ump_get_32_to_64(ump);
}
// either per-note or channel
LIBREMIDI_STATIC uint32_t cmidi2_ump_get_midi2_pitch_bend_data(const cmidi2_ump* ump)
{
  return cmidi2_ump_get_32_to_64(ump);
}
LIBREMIDI_STATIC uint8_t cmidi2_ump_get_midi2_pn_pitch_bend_note(const cmidi2_ump* ump)
{
  return cmidi2_ump_get_byte_at(ump, 2);
}

LIBREMIDI_STATIC uint8_t cmidi2_ump_get_sysex8_num_bytes(const cmidi2_ump* ump)
{
  return cmidi2_ump_get_channel(ump); // same bits
}
LIBREMIDI_STATIC uint8_t cmidi2_ump_get_sysex8_stream_id(const cmidi2_ump* ump)
{
  return cmidi2_ump_get_byte_at(ump, 2);
}
LIBREMIDI_STATIC uint8_t cmidi2_ump_get_mds_mds_id(const cmidi2_ump* ump)
{
  return cmidi2_ump_get_channel(ump); // same bits
}
LIBREMIDI_STATIC uint16_t cmidi2_ump_get_mds_num_chunk_bytes(const cmidi2_ump* ump)
{
  return (cmidi2_ump_get_byte_at(ump, 2) << 8) + cmidi2_ump_get_byte_at(ump, 3);
}
LIBREMIDI_STATIC uint16_t cmidi2_ump_get_mds_num_chunks(const cmidi2_ump* ump)
{
  return (cmidi2_ump_get_byte_at(ump, 4) << 8) + cmidi2_ump_get_byte_at(ump, 5);
}
LIBREMIDI_STATIC uint16_t cmidi2_ump_get_mds_chunk_index(const cmidi2_ump* ump)
{
  return (cmidi2_ump_get_byte_at(ump, 6) << 8) + cmidi2_ump_get_byte_at(ump, 7);
}
LIBREMIDI_STATIC uint16_t cmidi2_ump_get_mds_manufacturer_id(const cmidi2_ump* ump)
{
  return (cmidi2_ump_get_byte_at(ump, 8) << 8) + cmidi2_ump_get_byte_at(ump, 9);
}
LIBREMIDI_STATIC uint16_t cmidi2_ump_get_mds_device_id(const cmidi2_ump* ump)
{
  return (cmidi2_ump_get_byte_at(ump, 10) << 8) + cmidi2_ump_get_byte_at(ump, 11);
}
LIBREMIDI_STATIC uint16_t cmidi2_ump_get_mds_sub_id_1(const cmidi2_ump* ump)
{
  return (cmidi2_ump_get_byte_at(ump, 12) << 8) + cmidi2_ump_get_byte_at(ump, 13);
}
LIBREMIDI_STATIC uint16_t cmidi2_ump_get_mds_sub_id_2(const cmidi2_ump* ump)
{
  return (cmidi2_ump_get_byte_at(ump, 14) << 8) + cmidi2_ump_get_byte_at(ump, 15);
}

// --------
// Realtime-safe SysEx8 binary reader
//
// The primary end-user facing function is `cmidi2_ump_get_sysex8_data()`, but the
// function signature needs some explanation as it requires some complicated processing.
//
// There can be multiple SysEx8 "streams" that simultaneously run within a UMP stream
// (e.g. sysex8 start packet for stream #1, sysex8 continue packet for stream #1,
//  sysex8 start packet for stream #2, sysex8 continue packet for stream #1,
//  sysex8 continue packet for stream #2, sysex8 complete packet for stream #3,
//  sysex8 end packet for stream #2, sysex8 end packet for strean #1 ...).
//
// Those stream state must be preserved during UMP parsing. It is represented as
// `cmidi2_ump_binary_read_state` struct.
//
// There are handful of usage scenarios where we...
// - want to handle them all
// - want to handle only relevant ones
// - can control what we generate and determine that only one stream can appear so
//   the context stream is always obvious
//
// To handle them all, we need "stream selector". There is a function type
// `cmidi2_ump_stream_selector_func`, and a function pointer is passed to the entry
// point `cmidi2_ump_get_sysex8_data()` function.
//
// Another control point is where we want to finish parsing sysex8. Whenever a relevant
// stream is parsed, `cmidi2_ump_get_syex8_data()` calls "continuity checker" function
// which is typed as `cmidi2_ump_binary_read_continuity_checker_func`.
// If this function returns true, then it completes parsing, returning the number of
// the parsed UMPs (in uint32_t length).
//
// See `testType5Messages_sysex8_reader_writer()` testcase for the actual usage example.
//

enum cmidi2_ump_binary_reader_result_code
{
  CMIDI2_BINARY_READER_RESULT_INCOMPLETE = 0,
  CMIDI2_BINARY_READER_RESULT_COMPLETE = 1,
  CMIDI2_BINARY_READER_RESULT_NO_SPACE = 2,
};

typedef struct cmidi2_ump_binary_read_state
{
  void* context;
  uint8_t* data;
  size_t dataCapacity;
  size_t dataSize;
  bool continueOnCompletion;
  enum cmidi2_ump_binary_reader_result_code resultCode;
} cmidi2_ump_binary_read_state;

LIBREMIDI_STATIC void cmidi2_ump_binary_read_state_reset(cmidi2_ump_binary_read_state* state)
{
  state->dataSize = 0;
  state->resultCode = CMIDI2_BINARY_READER_RESULT_INCOMPLETE;
}

LIBREMIDI_STATIC void cmidi2_ump_binary_read_state_init(
    cmidi2_ump_binary_read_state* state, void* context, uint8_t* dataBuffer, size_t dataCapacity,
    bool continueOnCompletion)
{
  state->context = context;
  state->data = dataBuffer;
  state->dataCapacity = dataCapacity;
  state->continueOnCompletion = continueOnCompletion;
  cmidi2_ump_binary_read_state_reset(state);
}

/// Binary stream selector for cmidi2_ump_get_sysex8_data() function.
/// If you do not support simultaneous streams, just return a reference to a fixed `cmidi2_ump_binary_read_state` instance.
/// If the targetSteramId does not indicate the streams the client handles, then it should return NULL.
/// Note that the resulting state must contain a non-null data and valid dataCapacity i.e. it must be already assigned.
/// Also note that the function implementation should not try to allocate a new instance of state
/// - otherwise it will break realtime safety.
typedef cmidi2_ump_binary_read_state* (*cmidi2_ump_stream_selector_func)(
    uint8_t targetStreamId, void* context);

/// Binary stream continuity checker for cmidi2_ump_get_sysex8_data() function.
/// If you do not support simultaneous streams, then it should reset stream state (dataSize etc.) when new stream starts.
/// Return true to continue parsing, otherwise return false. It is useful to determine whether it
/// should break or not when a stream is fully read (status code is 3 = CMIDI2_SYSEX_END).
/// `cmidi2_ump_get_sysex8_data()` can take NULL for this function pointer at `continuityChecker` argument,
/// then it falls back to the "default" behavior.
/// By default, it finishes parsing when a stream completed and `continueOnCompletion` was `false`.
typedef bool (*cmidi2_ump_binary_read_continuity_checker_func)(
    cmidi2_ump_binary_read_state* stream, cmidi2_ump* ump);

// it is a special copy function that only works with sysex8 memory state at `src` that can access beyond `sizeInBytes`.
LIBREMIDI_STATIC void cmidi2_internal_sysex8_copy_data_byte_swapping(
    uint8_t* dst, uint8_t srcHead, uint32_t* srcTail, size_t sizeInBytes)
{
  if (sizeInBytes == 0)
    return;

  // the first byte is at the lowest byte so it is always safe to copy as is.
  dst[0] = srcHead;
  uint8_t* d = dst + 1;
  uint32_t* s = srcTail;
  sizeInBytes--;

  // copy the rest
  size_t i = 0;
  while (i + 3 < sizeInBytes)
  {
    uint32_t i32 = *s;
    d[i++] = i32 >> 24;
    d[i++] = (i32 >> 16) & 0xFF;
    d[i++] = (i32 >> 8) & 0xFF;
    d[i++] = i32 & 0xFF;
    s++;
  }
  for (; i < sizeInBytes; i++)
    d[i] = s[i + 3 - i % 4];
}

/// Parse and store sysex8 binary, using some fine-tuned behavioral functions.
/// Return the number of 32-bit ints (number of `cmidi2_ump`s)
LIBREMIDI_STATIC size_t cmidi2_ump_get_sysex8_data(
    cmidi2_ump_stream_selector_func streamSelector, void* streamSelectorContext,
    cmidi2_ump_binary_read_continuity_checker_func continuityChecker, const cmidi2_ump* ump,
    const size_t umpCapacityInInt)
{

  cmidi2_ump *umpPtr = (cmidi2_ump*)ump, *umpEnd = (cmidi2_ump*)ump + umpCapacityInInt;
  for (; umpPtr < umpEnd; umpPtr += cmidi2_ump_get_message_size_bytes(umpPtr) / sizeof(cmidi2_ump))
  {
    if (cmidi2_ump_get_message_type(umpPtr) != CMIDI2_MESSAGE_TYPE_SYSEX8_MDS)
      continue;
    switch (cmidi2_ump_get_status_code(umpPtr))
    {
      case CMIDI2_SYSEX_IN_ONE_UMP:
      case CMIDI2_SYSEX_START:
      case CMIDI2_SYSEX_END:
      case CMIDI2_SYSEX_CONTINUE:
        break;
      default:
        continue; // the only expected value here is MDS (8 or 9)
    }

    cmidi2_ump_binary_read_state* state
        = streamSelector(cmidi2_ump_get_sysex8_stream_id(umpPtr), streamSelectorContext);
    if (state == NULL)
      continue;

    size_t copySize = cmidi2_ump_get_sysex8_num_bytes(umpPtr)
                      - 1; // SysEx8 size field contains the size byte itself, hence -1.
    if (state->dataSize + copySize >= state->dataCapacity)
    {
      state->resultCode = CMIDI2_BINARY_READER_RESULT_NO_SPACE;
      return umpPtr - ump;
    }

    if (cmidi2_util_is_platform_little_endian())
      // We need to swap data along with byte order for resulting data...
      cmidi2_internal_sysex8_copy_data_byte_swapping(
          state->data + state->dataSize, (*umpPtr) & 0xFF, umpPtr + 1, copySize);
    else
      memcpy(state->data + state->dataSize, ((uint8_t*)(void*)umpPtr) + 3, copySize);

    state->dataSize += copySize;

    if (continuityChecker != NULL)
      if (continuityChecker(state, umpPtr))
        return umpPtr + cmidi2_ump_get_message_size_bytes(umpPtr) / sizeof(cmidi2_ump)
               - ump; // "break here" is indicated

    // otherwise default continuity checker
    switch (cmidi2_ump_get_status_code(umpPtr))
    {
      case CMIDI2_SYSEX_IN_ONE_UMP:
      case CMIDI2_SYSEX_END:
        state->resultCode = CMIDI2_BINARY_READER_RESULT_COMPLETE;
        if (state->continueOnCompletion)
          return umpPtr + cmidi2_ump_get_message_size_bytes(umpPtr) / sizeof(cmidi2_ump)
                 - ump; // default "break here" condition.
        break;
    }
  }
  // finished parsing while no stream indicated "break here" for completion.
  return umpPtr - ump;
}

// --------
// sequence iterator

/* byte stream splitter */

LIBREMIDI_STATIC void* cmidi2_ump_sequence_next(const void* ptr)
{
  return (uint8_t*)ptr + cmidi2_ump_get_num_bytes(cmidi2_ump_read_uint32_bytes(ptr));
}

// similar to LV2_ATOM Utilities API...
#define CMIDI2_UMP_SEQUENCE_FOREACH(ptr, numBytes, iter)                    \
  for (uint8_t*(iter) = (uint8_t*)ptr; (iter) < ((uint8_t*)ptr) + numBytes; \
       (iter) = (uint8_t*)cmidi2_ump_sequence_next(iter))

LIBREMIDI_STATIC void* cmidi2_ump_sequence_next_le(const void* ptr)
{
  return (uint8_t*)ptr + cmidi2_ump_get_num_bytes(cmidi2_ump_read_uint32_bytes_le(ptr));
}

#define CMIDI2_UMP_SEQUENCE_FOREACH_LE(ptr, numBytes, iter)                 \
  for (uint8_t*(iter) = (uint8_t*)ptr; (iter) < ((uint8_t*)ptr) + numBytes; \
       (iter) = (uint8_t*)cmidi2_ump_sequence_next_le(iter))

LIBREMIDI_STATIC void* cmidi2_ump_sequence_next_be(const void* ptr)
{
  return (uint8_t*)ptr + cmidi2_ump_get_num_bytes(cmidi2_ump_read_uint32_bytes_be(ptr));
}

#define CMIDI2_UMP_SEQUENCE_FOREACH_BE(ptr, numBytes, iter)                 \
  for (uint8_t*(iter) = (uint8_t*)ptr; (iter) < ((uint8_t*)ptr) + numBytes; \
       (iter) = (uint8_t*)cmidi2_ump_sequence_next_be(iter))

// --------
// MIDI CI support.

#define CMIDI2_CI_SUB_ID 0xD
#define CMIDI2_CI_SUB_ID_2_DISCOVERY_INQUIRY 0x70
#define CMIDI2_CI_SUB_ID_2_DISCOVERY_REPLY 0x71
#define CMIDI2_CI_SUB_ID_2_INVALIDATE_MUID 0x7E
#define CMIDI2_CI_SUB_ID_2_ACK 0x7F
#define CMIDI2_CI_SUB_ID_2_NAK 0x7F
#define CMIDI2_CI_SUB_ID_2_PROTOCOL_NEGOTIATION_INQUIRY 0x10
#define CMIDI2_CI_SUB_ID_2_PROTOCOL_NEGOTIATION_REPLY 0x11
#define CMIDI2_CI_SUB_ID_2_SET_NEW_PROTOCOL 0x12
#define CMIDI2_CI_SUB_ID_2_TEST_NEW_PROTOCOL_I2R 0x13
#define CMIDI2_CI_SUB_ID_2_TEST_NEW_PROTOCOL_R2I 0x14
#define CMIDI2_CI_SUB_ID_2_CONFIRM_NEW_PROTOCOL_ESTABLISHED 0x15
#define CMIDI2_CI_SUB_ID_2_PROFILE_INQUIRY 0x20
#define CMIDI2_CI_SUB_ID_2_PROFILE_INQUIRY_REPLY 0x21
#define CMIDI2_CI_SUB_ID_2_SET_PROFILE_ON 0x22
#define CMIDI2_CI_SUB_ID_2_SET_PROFILE_OFF 0x23
#define CMIDI2_CI_SUB_ID_2_PROFILE_ENABLED_REPORT 0x24
#define CMIDI2_CI_SUB_ID_2_PROFILE_DISABLED_REPORT 0x25
#define CMIDI2_CI_SUB_ID_2_PROFILE_SPECIFIC_DATA 0x2F
#define CMIDI2_CI_SUB_ID_2_PROPERTY_CAPABILITIES_INQUIRY 0x30
#define CMIDI2_CI_SUB_ID_2_PROPERTY_CAPABILITIES_REPLY 0x31
#define CMIDI2_CI_SUB_ID_2_PROPERTY_HAS_DATA 0x32
#define CMIDI2_CI_SUB_ID_2_PROPERTY_HAS_DATA_REPLY 0x33
#define CMIDI2_CI_SUB_ID_2_PROPERTY_GET_DATA 0x34
#define CMIDI2_CI_SUB_ID_2_PROPERTY_GET_DATA_REPLY 0x35
#define CMIDI2_CI_SUB_ID_2_PROPERTY_SET_DATA 0x36
#define CMIDI2_CI_SUB_ID_2_PROPERTY_SET_DATA_REPLY 0x37
#define CMIDI2_CI_SUB_ID_2_PROPERTY_SUBSCRIBE 0x38
#define CMIDI2_CI_SUB_ID_2_PROPERTY_SUBSCRIBE_REPLY 0x39
#define CMIDI2_CI_SUB_ID_2_PROPERTY_NOTIFY 0x3F
#define CMIDI2_CI_SUB_ID_2_PROCESS_GET_CAPABILITIES 0x40
#define CMIDI2_CI_SUB_ID_2_PROCESS_GET_CAPABILITIES_REPLY 0x41
#define CMIDI2_CI_SUB_ID_2_PROCESS_GET_MIDI_REPORT 0x42
#define CMIDI2_CI_SUB_ID_2_PROCESS_GET_MIDI_REPORT_REPLY 0x43
#define CMIDI2_CI_SUB_ID_2_PROCESS_GET_MIDI_REPORT_END 0x44

#define CMIDI2_CI_PROTOCOL_NEGOTIATION_SUPPORTED 2
#define CMIDI2_CI_PROFILE_CONFIGURATION_SUPPORTED 4
#define CMIDI2_CI_PROPERTY_EXCHANGE_SUPPORTED 8

#define CMIDI2_CI_DEVICE_ID_WHOLE_FUNCTION_BLOCK 0x7F

typedef struct
{
  uint8_t type;
  uint8_t version;
  uint8_t extensions;
  uint8_t reserved1;
  uint8_t reserved2;
} cmidi2_ci_protocol_type_info;

typedef struct
{
  uint8_t fixed_7e; // 0x7E
  uint8_t bank;
  uint8_t number;
  uint8_t version;
  uint8_t level;
} cmidi2_profile_id;

// Assumes the input value is already 7-bit encoded if required.
LIBREMIDI_STATIC void cmidi2_ci_direct_uint16_at(uint8_t* buf, uint16_t v)
{
  buf[0] = v & 0xFF;
  buf[1] = (v >> 8) & 0xFF;
}

// Assumes the input value is already 7-bit encoded if required.
LIBREMIDI_STATIC void cmidi2_ci_direct_uint32_at(uint8_t* buf, uint32_t v)
{
  buf[0] = v & 0xFF;
  buf[1] = (v >> 8) & 0xFF;
  buf[2] = (v >> 16) & 0xFF;
  buf[3] = (v >> 24) & 0xFF;
}

LIBREMIDI_STATIC void cmidi2_ci_7bit_int14_at(uint8_t* buf, uint16_t v)
{
  buf[0] = v & 0x7F;
  buf[1] = (v >> 7) & 0x7F;
}

LIBREMIDI_STATIC void cmidi2_ci_7bit_int21_at(uint8_t* buf, uint32_t v)
{
  buf[0] = v & 0x7F;
  buf[1] = (v >> 7) & 0x7F;
  buf[2] = (v >> 14) & 0x7F;
}

LIBREMIDI_STATIC void cmidi2_ci_7bit_int28_at(uint8_t* buf, uint32_t v)
{
  buf[0] = v & 0x7F;
  buf[1] = (v >> 7) & 0x7F;
  buf[2] = (v >> 14) & 0x7F;
  buf[3] = (v >> 21) & 0x7F;
}

LIBREMIDI_STATIC void cmidi2_ci_message_common(
    uint8_t* buf, uint8_t destination, uint8_t sysexSubId2, uint8_t versionAndFormat,
    uint32_t sourceMUID, uint32_t destinationMUID)
{
  buf[0] = 0x7E;
  buf[1] = destination;
  buf[2] = CMIDI2_CI_SUB_ID;
  buf[3] = sysexSubId2;
  buf[4] = versionAndFormat;
  cmidi2_ci_direct_uint32_at(buf + 5, sourceMUID);
  cmidi2_ci_direct_uint32_at(buf + 9, destinationMUID);
}

// Discovery

LIBREMIDI_STATIC void cmidi2_ci_discovery_common(
    uint8_t* buf, uint8_t sysexSubId2, uint8_t versionAndFormat, uint32_t sourceMUID,
    uint32_t destinationMUID, uint32_t deviceManufacturer3Bytes, uint16_t deviceFamily,
    uint16_t deviceFamilyModelNumber, uint32_t softwareRevisionLevel, uint8_t ciCategorySupported,
    uint32_t receivableMaxSysExSize, uint8_t initiatorOutputPathId)
{
  cmidi2_ci_message_common(
      buf, CMIDI2_CI_DEVICE_ID_WHOLE_FUNCTION_BLOCK, sysexSubId2, versionAndFormat, sourceMUID,
      destinationMUID);
  cmidi2_ci_direct_uint32_at(
      buf + 13,
      deviceManufacturer3Bytes); // the last byte is extraneous, but will be overwritten next.
  cmidi2_ci_direct_uint16_at(buf + 16, deviceFamily);
  cmidi2_ci_direct_uint16_at(buf + 18, deviceFamilyModelNumber);
  // LAMESPEC: Software Revision Level does not mention in which endianness this field is stored.
  cmidi2_ci_direct_uint32_at(buf + 20, softwareRevisionLevel);
  buf[24] = ciCategorySupported;
  cmidi2_ci_direct_uint32_at(buf + 25, receivableMaxSysExSize);
  buf[29] = initiatorOutputPathId;
}

LIBREMIDI_STATIC void cmidi2_ci_discovery(
    uint8_t* buf, uint8_t versionAndFormat, uint32_t sourceMUID, uint32_t deviceManufacturer,
    uint16_t deviceFamily, uint16_t deviceFamilyModelNumber, uint32_t softwareRevisionLevel,
    uint8_t ciCategorySupported, uint32_t receivableMaxSysExSize, uint8_t initiatorOutputPathId)
{
  cmidi2_ci_discovery_common(
      buf, CMIDI2_CI_SUB_ID_2_DISCOVERY_INQUIRY, versionAndFormat, sourceMUID, 0x7F7F7F7F,
      deviceManufacturer, deviceFamily, deviceFamilyModelNumber, softwareRevisionLevel,
      ciCategorySupported, receivableMaxSysExSize, initiatorOutputPathId);
}

LIBREMIDI_STATIC void cmidi2_ci_discovery_reply(
    uint8_t* buf, uint8_t versionAndFormat, uint32_t sourceMUID, uint32_t destinationMUID,
    uint32_t deviceManufacturer, uint16_t deviceFamily, uint16_t deviceFamilyModelNumber,
    uint32_t softwareRevisionLevel, uint8_t ciCategorySupported, uint32_t receivableMaxSysExSize,
    uint8_t initiatorOutputPathId, uint8_t functionBlockOr7Fh)
{
  cmidi2_ci_discovery_common(
      buf, CMIDI2_CI_SUB_ID_2_DISCOVERY_REPLY, versionAndFormat, sourceMUID, destinationMUID,
      deviceManufacturer, deviceFamily, deviceFamilyModelNumber, softwareRevisionLevel,
      ciCategorySupported, receivableMaxSysExSize, initiatorOutputPathId);
  buf[30] = functionBlockOr7Fh;
}

LIBREMIDI_STATIC void cmidi2_ci_discovery_invalidate_muid(
    uint8_t* buf, uint8_t versionAndFormat, uint32_t sourceMUID, uint32_t targetMUID)
{
  cmidi2_ci_message_common(
      buf, 0x7F, CMIDI2_CI_SUB_ID_2_INVALIDATE_MUID, versionAndFormat, sourceMUID, 0x7F7F7F7F);
  cmidi2_ci_direct_uint32_at(buf + 13, targetMUID);
}

LIBREMIDI_STATIC void cmidi2_ci_ack_nak_common(
    uint8_t* buf, uint8_t deviceId, uint8_t versionAndFormat, uint32_t sourceMUID,
    uint32_t destinationMUID, uint8_t originalTransactionSubID2Class, uint8_t statusCode,
    uint8_t statusData, uint8_t* details5Bytes, uint16_t messageLength, const char* messageText)
{
  cmidi2_ci_message_common(
      buf, deviceId, CMIDI2_CI_SUB_ID_2_ACK, versionAndFormat, sourceMUID, destinationMUID);
  buf[13] = originalTransactionSubID2Class;
  buf[14] = statusCode;
  buf[15] = statusData;
  memcpy(buf + 16, details5Bytes, 5);
  cmidi2_ci_direct_uint16_at(buf + 21, messageLength);
  memcpy(buf + 23, messageText, messageLength);
}

LIBREMIDI_STATIC void cmidi2_ci_ack(
    uint8_t* buf, uint8_t deviceId, uint8_t versionAndFormat, uint32_t sourceMUID,
    uint32_t destinationMUID, uint8_t originalTransactionSubID2Class, uint8_t ackStatusCode,
    uint8_t ackStatusData, uint8_t* ackDetails5Bytes, uint16_t messageLength,
    const char* messageText)
{
  cmidi2_ci_ack_nak_common(
      buf, deviceId, versionAndFormat, sourceMUID, destinationMUID, originalTransactionSubID2Class,
      ackStatusCode, ackStatusData, ackDetails5Bytes, messageLength, messageText);
}

LIBREMIDI_STATIC void cmidi2_ci_nak(
    uint8_t* buf, uint8_t deviceId, uint8_t versionAndFormat, uint32_t sourceMUID,
    uint32_t destinationMUID, uint8_t originalTransactionSubID2Class, uint8_t nakStatusCode,
    uint8_t nakStatusData, uint8_t* nakDetails5Bytes, uint16_t messageLength,
    const char* messageText)
{
  cmidi2_ci_ack_nak_common(
      buf, deviceId, versionAndFormat, sourceMUID, destinationMUID, originalTransactionSubID2Class,
      nakStatusCode, nakStatusData, nakDetails5Bytes, messageLength, messageText);
}

// Protocol Negotiation
// Note that it was removed in MIDI 2.0 specification June 2023 Updates (MIDI-CI 1.2).

LIBREMIDI_STATIC void cmidi2_ci_protocol_info(uint8_t* buf, cmidi2_ci_protocol_type_info info)
{
  buf[0] = info.type;
  buf[1] = info.version;
  buf[2] = info.extensions;
  buf[3] = info.reserved1;
  buf[4] = info.reserved2;
}

LIBREMIDI_STATIC void cmidi2_ci_protocols(
    uint8_t* buf, uint8_t numSupportedProtocols, cmidi2_ci_protocol_type_info* protocolTypes)
{
  buf[0] = numSupportedProtocols;
  for (int i = 0; i < numSupportedProtocols; i++)
    cmidi2_ci_protocol_info(buf + 1 + i * 5, protocolTypes[i]);
}

LIBREMIDI_STATIC void cmidi2_ci_protocol_negotiation(
    uint8_t* buf, bool isReply, uint32_t sourceMUID, uint32_t destinationMUID,
    uint8_t authorityLevel, uint8_t numSupportedProtocols,
    cmidi2_ci_protocol_type_info* protocolTypes)
{
  cmidi2_ci_message_common(
      buf, 0x7F,
      isReply ? CMIDI2_CI_SUB_ID_2_PROTOCOL_NEGOTIATION_REPLY
              : CMIDI2_CI_SUB_ID_2_PROTOCOL_NEGOTIATION_INQUIRY,
      1, sourceMUID, destinationMUID);
  buf[13] = authorityLevel;
  cmidi2_ci_protocols(buf + 14, numSupportedProtocols, protocolTypes);
}

LIBREMIDI_STATIC void cmidi2_ci_protocol_set(
    uint8_t* buf, uint32_t sourceMUID, uint32_t destinationMUID, uint8_t authorityLevel,
    cmidi2_ci_protocol_type_info newProtocolType)
{
  cmidi2_ci_message_common(
      buf, 0x7F, CMIDI2_CI_SUB_ID_2_SET_NEW_PROTOCOL, 1, sourceMUID, destinationMUID);
  buf[13] = authorityLevel;
  cmidi2_ci_protocol_info(buf + 14, newProtocolType);
}

LIBREMIDI_STATIC void cmidi2_ci_protocol_test(
    uint8_t* buf, bool isInitiatorToResponder, uint32_t sourceMUID, uint32_t destinationMUID,
    uint8_t authorityLevel, uint8_t* testData48Bytes)
{
  cmidi2_ci_message_common(
      buf, 0x7F,
      isInitiatorToResponder ? CMIDI2_CI_SUB_ID_2_TEST_NEW_PROTOCOL_I2R
                             : CMIDI2_CI_SUB_ID_2_TEST_NEW_PROTOCOL_R2I,
      1, sourceMUID, destinationMUID);
  buf[13] = authorityLevel;
  memcpy(buf + 14, testData48Bytes, 48);
}

LIBREMIDI_STATIC void cmidi2_ci_protocol_confirm_established(
    uint8_t* buf, uint32_t sourceMUID, uint32_t destinationMUID, uint8_t authorityLevel)
{
  cmidi2_ci_message_common(
      buf, 0x7F, CMIDI2_CI_SUB_ID_2_CONFIRM_NEW_PROTOCOL_ESTABLISHED, 1, sourceMUID,
      destinationMUID);
  buf[13] = authorityLevel;
}

LIBREMIDI_STATIC int32_t cmidi2_ci_try_parse_new_protocol(uint8_t* buf, size_t length)
{
  return (length != 19 || buf[0] != 0x7E || buf[1] != 0x7F || buf[2] != CMIDI2_CI_SUB_ID
          || buf[3] != CMIDI2_CI_SUB_ID_2_SET_NEW_PROTOCOL || buf[4] != 1)
             ? 0
             : buf[14];
}

// Profile Configuration

LIBREMIDI_STATIC void cmidi2_ci_profile(uint8_t* buf, cmidi2_profile_id info)
{
  buf[0] = info.fixed_7e;
  buf[1] = info.bank;
  buf[2] = info.number;
  buf[3] = info.version;
  buf[4] = info.level;
}

LIBREMIDI_STATIC void cmidi2_ci_profile_inquiry(
    uint8_t* buf, uint8_t source, uint32_t sourceMUID, uint32_t destinationMUID)
{
  cmidi2_ci_message_common(
      buf, source, CMIDI2_CI_SUB_ID_2_PROFILE_INQUIRY, 1, sourceMUID, destinationMUID);
}

LIBREMIDI_STATIC void cmidi2_ci_profile_inquiry_reply(
    uint8_t* buf, uint8_t source, uint32_t sourceMUID, uint32_t destinationMUID,
    uint8_t numEnabledProfiles, cmidi2_profile_id* enabledProfiles, uint8_t numDisabledProfiles,
    cmidi2_profile_id* disabledProfiles)
{
  cmidi2_ci_message_common(
      buf, source, CMIDI2_CI_SUB_ID_2_PROFILE_INQUIRY_REPLY, 1, sourceMUID, destinationMUID);
  buf[13] = numEnabledProfiles;
  for (int i = 0; i < numEnabledProfiles; i++)
    cmidi2_ci_profile(buf + 14 + i * 5, enabledProfiles[i]);
  uint32_t pos = 14 + numEnabledProfiles * 5;
  buf[pos++] = numDisabledProfiles;
  for (int i = 0; i < numDisabledProfiles; i++)
    cmidi2_ci_profile(buf + pos + i * 5, disabledProfiles[i]);
}

LIBREMIDI_STATIC void cmidi2_ci_profile_set(
    uint8_t* buf, uint8_t destination, bool turnOn, uint32_t sourceMUID, uint32_t destinationMUID,
    cmidi2_profile_id profile)
{
  cmidi2_ci_message_common(
      buf, destination,
      turnOn ? CMIDI2_CI_SUB_ID_2_SET_PROFILE_ON : CMIDI2_CI_SUB_ID_2_SET_PROFILE_OFF, 1,
      sourceMUID, destinationMUID);
  cmidi2_ci_profile(buf + 13, profile);
}

LIBREMIDI_STATIC void cmidi2_ci_profile_report(
    uint8_t* buf, uint8_t source, bool isEnabledReport, uint32_t sourceMUID,
    cmidi2_profile_id profile)
{
  cmidi2_ci_message_common(
      buf, source,
      isEnabledReport ? CMIDI2_CI_SUB_ID_2_PROFILE_ENABLED_REPORT
                      : CMIDI2_CI_SUB_ID_2_PROFILE_DISABLED_REPORT,
      1, sourceMUID, 0x7F7F7F7F);
  cmidi2_ci_profile(buf + 13, profile);
}

LIBREMIDI_STATIC void cmidi2_ci_profile_specific_data(
    uint8_t* buf, uint8_t source, uint32_t sourceMUID, uint32_t destinationMUID,
    cmidi2_profile_id profile, uint32_t dataSize, void* data)
{
  cmidi2_ci_message_common(
      buf, source, CMIDI2_CI_SUB_ID_2_PROFILE_SPECIFIC_DATA, 1, sourceMUID, destinationMUID);
  cmidi2_ci_profile(buf + 13, profile);
  cmidi2_ci_direct_uint32_at(buf + 18, dataSize);
  memcpy(buf + 22, data, dataSize);
}

// Property Exchange

LIBREMIDI_STATIC void cmidi2_ci_property_get_capabilities(
    uint8_t* buf, uint8_t destination, bool isReply, uint32_t sourceMUID, uint32_t destinationMUID,
    uint8_t maxSupportedRequests)
{
  cmidi2_ci_message_common(
      buf, destination,
      isReply ? CMIDI2_CI_SUB_ID_2_PROPERTY_CAPABILITIES_REPLY
              : CMIDI2_CI_SUB_ID_2_PROPERTY_CAPABILITIES_INQUIRY,
      1, sourceMUID, destinationMUID);
  buf[13] = maxSupportedRequests;
}

// common to all of: has data & reply, get data & reply, set data & reply, subscribe & reply, notify
LIBREMIDI_STATIC void cmidi2_ci_property_common(
    uint8_t* buf, uint8_t destination, uint8_t messageTypeSubId2, uint32_t sourceMUID,
    uint32_t destinationMUID, uint8_t requestId, uint16_t headerSize, void* header,
    uint16_t numChunks, uint16_t chunkIndex, uint16_t dataSize, void* data)
{
  cmidi2_ci_message_common(buf, destination, messageTypeSubId2, 1, sourceMUID, destinationMUID);
  buf[13] = requestId;
  cmidi2_ci_direct_uint16_at(buf + 14, headerSize);
  memcpy(buf + 16, header, headerSize);
  cmidi2_ci_direct_uint16_at(buf + 16 + headerSize, numChunks);
  cmidi2_ci_direct_uint16_at(buf + 18 + headerSize, chunkIndex);
  cmidi2_ci_direct_uint16_at(buf + 20 + headerSize, dataSize);
  memcpy(buf + 22 + headerSize, data, dataSize);
}

LIBREMIDI_STATIC void cmidi2_ci_property_get_capabilities_reply(
    uint8_t* buf, uint8_t versionAndFormat, uint32_t sourceMUID, uint32_t destinationMUID,
    uint8_t maxSupportedRequests, uint8_t peMajorVersion, uint8_t peMinorVersion)
{
  cmidi2_ci_message_common(
      buf, CMIDI2_CI_DEVICE_ID_WHOLE_FUNCTION_BLOCK,
      CMIDI2_CI_SUB_ID_2_PROPERTY_CAPABILITIES_REPLY, versionAndFormat, sourceMUID,
      destinationMUID);
  buf[13] = maxSupportedRequests;
  buf[14] = peMajorVersion;
  buf[15] = peMinorVersion;
}

LIBREMIDI_STATIC void cmidi2_ci_property_data_common(
    uint8_t* buf, uint8_t subId2, uint8_t versionAndFormat, uint32_t sourceMUID,
    uint32_t destinationMUID, uint8_t requestId, size_t headerSize, const uint8_t* headerData,
    uint16_t numChunksInMessage, uint16_t currentChunk, uint16_t propertyDataLength,
    const char* propertyData)
{
  cmidi2_ci_message_common(
      buf, CMIDI2_CI_DEVICE_ID_WHOLE_FUNCTION_BLOCK, subId2, versionAndFormat, sourceMUID,
      destinationMUID);
  buf[13] = requestId;
  buf[14] = headerSize % 0xFF;
  buf[15] = headerSize / 0xFF;
  memcpy(buf + 16, headerData, headerSize);
  cmidi2_ci_direct_uint16_at(buf + headerSize + 16, numChunksInMessage);
  cmidi2_ci_direct_uint16_at(buf + headerSize + 18, currentChunk);
  cmidi2_ci_direct_uint16_at(buf + headerSize + 20, propertyDataLength);
  if (propertyData)
    memcpy(buf + headerSize + 22, propertyData, propertyDataLength);
}

LIBREMIDI_STATIC void cmidi2_ci_property_get_data(
    uint8_t* buf, uint8_t versionAndFormat, uint32_t sourceMUID, uint32_t destinationMUID,
    uint8_t requestId, size_t headerSize, const uint8_t* headerData)
{
  cmidi2_ci_property_data_common(
      buf, CMIDI2_CI_SUB_ID_2_PROPERTY_GET_DATA, versionAndFormat, sourceMUID, destinationMUID,
      requestId, headerSize, headerData, 1, 1, 0, NULL);
}

LIBREMIDI_STATIC void cmidi2_ci_property_get_data_reply(
    uint8_t* buf, uint8_t versionAndFormat, uint32_t sourceMUID, uint32_t destinationMUID,
    uint8_t requestId, size_t headerSize, const uint8_t* headerData, uint16_t numChunksInMessage,
    uint16_t currentChunk, uint16_t propertyDataLength, const char* propertyData)
{
  cmidi2_ci_property_data_common(
      buf, CMIDI2_CI_SUB_ID_2_PROPERTY_GET_DATA_REPLY, versionAndFormat, sourceMUID,
      destinationMUID, requestId, headerSize, headerData, numChunksInMessage, currentChunk,
      propertyDataLength, propertyData);
}

LIBREMIDI_STATIC void cmidi2_ci_property_set_data(
    uint8_t* buf, uint8_t versionAndFormat, uint32_t sourceMUID, uint32_t destinationMUID,
    uint8_t requestId, size_t headerSize, const uint8_t* headerData, uint16_t numChunksInMessage,
    uint16_t currentChunk, uint16_t propertyDataLength, const char* propertyData)
{
  cmidi2_ci_property_data_common(
      buf, CMIDI2_CI_SUB_ID_2_PROPERTY_SET_DATA, versionAndFormat, sourceMUID, destinationMUID,
      requestId, headerSize, headerData, numChunksInMessage, currentChunk, propertyDataLength,
      propertyData);
}

LIBREMIDI_STATIC void cmidi2_ci_property_set_data_reply(
    uint8_t* buf, uint8_t versionAndFormat, uint32_t sourceMUID, uint32_t destinationMUID,
    uint8_t requestId, size_t headerSize, const uint8_t* headerData)
{
  cmidi2_ci_property_data_common(
      buf, CMIDI2_CI_SUB_ID_2_PROPERTY_SET_DATA_REPLY, versionAndFormat, sourceMUID,
      destinationMUID, requestId, headerSize, headerData, 1, 1, 0, NULL);
}

LIBREMIDI_STATIC void cmidi2_ci_property_subscribe(
    uint8_t* buf, uint8_t versionAndFormat, uint32_t sourceMUID, uint32_t destinationMUID,
    uint8_t requestId, size_t headerSize, const uint8_t* headerData, uint16_t numChunksInMessage,
    uint16_t currentChunk, uint16_t propertyDataLength, const char* propertyData)
{
  cmidi2_ci_property_data_common(
      buf, CMIDI2_CI_SUB_ID_2_PROPERTY_SUBSCRIBE, versionAndFormat, sourceMUID, destinationMUID,
      requestId, headerSize, headerData, numChunksInMessage, currentChunk, propertyDataLength,
      propertyData);
}

LIBREMIDI_STATIC void cmidi2_ci_property_subscribe_reply(
    uint8_t* buf, uint8_t versionAndFormat, uint32_t sourceMUID, uint32_t destinationMUID,
    uint8_t requestId, size_t headerSize, const uint8_t* headerData, uint16_t numChunksInMessage,
    uint16_t currentChunk, uint16_t propertyDataLength, const char* propertyData)
{
  cmidi2_ci_property_data_common(
      buf, CMIDI2_CI_SUB_ID_2_PROPERTY_SUBSCRIBE_REPLY, versionAndFormat, sourceMUID,
      destinationMUID, requestId, headerSize, headerData, numChunksInMessage, currentChunk,
      propertyDataLength, propertyData);
}

LIBREMIDI_STATIC void cmidi2_ci_property_notify(
    uint8_t* buf, uint8_t versionAndFormat, uint32_t sourceMUID, uint32_t destinationMUID,
    uint8_t requestId, size_t headerSize, const uint8_t* headerData, uint16_t numChunksInMessage,
    uint16_t currentChunk, uint16_t propertyDataLength, const char* propertyData)
{
  cmidi2_ci_property_data_common(
      buf, CMIDI2_CI_SUB_ID_2_PROPERTY_NOTIFY, versionAndFormat, sourceMUID, destinationMUID,
      requestId, headerSize, headerData, numChunksInMessage, currentChunk, propertyDataLength,
      propertyData);
}

// Process Inquiry

LIBREMIDI_STATIC void cmidi2_ci_process_get_capabilities(
    uint8_t* buf, uint8_t subId2, uint8_t versionAndFormat, uint32_t sourceMUID,
    uint32_t destinationMUID)
{
  (void)subId2;
  cmidi2_ci_message_common(
      buf, CMIDI2_CI_DEVICE_ID_WHOLE_FUNCTION_BLOCK, CMIDI2_CI_SUB_ID_2_PROCESS_GET_CAPABILITIES,
      versionAndFormat, sourceMUID, destinationMUID);
}

LIBREMIDI_STATIC void cmidi2_ci_process_get_capabilities_reply(
    uint8_t* buf, uint8_t subId2, uint8_t versionAndFormat, uint32_t sourceMUID,
    uint32_t destinationMUID, uint8_t processInquirySupportedFeatures)
{
  (void)subId2;
  cmidi2_ci_message_common(
      buf, CMIDI2_CI_DEVICE_ID_WHOLE_FUNCTION_BLOCK,
      CMIDI2_CI_SUB_ID_2_PROCESS_GET_CAPABILITIES_REPLY, versionAndFormat, sourceMUID,
      destinationMUID);
  buf[13] = processInquirySupportedFeatures;
}

LIBREMIDI_STATIC void cmidi2_ci_process_midi_report_common(
    uint8_t* buf, uint8_t deviceId, uint8_t subId2, uint8_t versionAndFormat, uint32_t sourceMUID,
    uint32_t destinationMUID, uint8_t messageDataControl, uint8_t requestedSystemMessages,
    uint8_t requestedChannelControllerMessages, uint8_t requestedNoteDataMessages)
{
  (void)subId2;
  cmidi2_ci_message_common(
      buf, deviceId, CMIDI2_CI_SUB_ID_2_PROCESS_GET_CAPABILITIES, versionAndFormat, sourceMUID,
      destinationMUID);
  buf[13] = messageDataControl;
  buf[14] = requestedSystemMessages;
  buf[15] = 0; // reserved
  buf[16] = requestedChannelControllerMessages;
  buf[17] = requestedNoteDataMessages;
}

LIBREMIDI_STATIC void cmidi2_ci_process_get_midi_report(
    uint8_t* buf, uint8_t deviceId, uint8_t versionAndFormat, uint32_t sourceMUID,
    uint32_t destinationMUID, uint8_t messageDataControl, uint8_t requestedSystemMessages,
    uint8_t requestedChannelControllerMessages, uint8_t requestedNoteDataMessages)
{
  cmidi2_ci_process_midi_report_common(
      buf, deviceId, CMIDI2_CI_SUB_ID_2_PROCESS_GET_MIDI_REPORT, versionAndFormat, sourceMUID,
      destinationMUID, messageDataControl, requestedSystemMessages,
      requestedChannelControllerMessages, requestedNoteDataMessages);
}

LIBREMIDI_STATIC void cmidi2_ci_process_get_midi_report_reply(
    uint8_t* buf, uint8_t deviceId, uint8_t versionAndFormat, uint32_t sourceMUID,
    uint32_t destinationMUID, uint8_t messageDataControl, uint8_t requestedSystemMessages,
    uint8_t requestedChannelControllerMessages, uint8_t requestedNoteDataMessages)
{
  cmidi2_ci_process_midi_report_common(
      buf, deviceId, CMIDI2_CI_SUB_ID_2_PROCESS_GET_MIDI_REPORT_REPLY, versionAndFormat,
      sourceMUID, destinationMUID, messageDataControl, requestedSystemMessages,
      requestedChannelControllerMessages, requestedNoteDataMessages);
}

LIBREMIDI_STATIC void cmidi2_ci_process_get_midi_report_end(
    uint8_t* buf, uint8_t deviceId, uint8_t versionAndFormat, uint32_t sourceMUID,
    uint32_t destinationMUID)
{
  cmidi2_ci_message_common(
      buf, deviceId, CMIDI2_CI_SUB_ID_2_PROCESS_GET_MIDI_REPORT_END, versionAndFormat, sourceMUID,
      destinationMUID);
}

// Miscellaneous MIDI Utilities

/** Encodes `value` into `bytes` stream. Returns the length of the encoded value in bytes. */
LIBREMIDI_STATIC uint8_t cmidi2_midi1_write_7bit_encoded_int(uint8_t* bytes, uint32_t value)
{
  uint8_t pos = 0;
  for (;; pos++)
  {
    bytes[pos] = value % 0x80;
    if (value >= 0x80)
    {
      value /= 0x80;
      bytes[pos] |= 0x80;
    }
    else
      return pos + 1;
  }
}

/** Returns the length of `value` when it would be encoded into a byte stream. */
LIBREMIDI_STATIC uint8_t cmidi2_midi1_get_7bit_encoded_int_length(uint32_t value)
{
  for (uint8_t ret = 1;; ret++)
  {
    if (value >= 0x80)
      value /= 0x80;
    else
      return ret;
  }
  return 0;
}

/** Returns the 7-bit encoded value in `bytes` stream of `length` bytes. */
LIBREMIDI_STATIC uint32_t cmidi2_midi1_get_7bit_encoded_int(uint8_t* bytes, uint32_t length)
{
  uint8_t* start = bytes;
  uint32_t value = 0;
  for (int digits = 0;; digits++)
  {
    if (bytes >= start + length)
      break;
    value += (0x7F & (*bytes)) << (digits * 7);
    if (*bytes < 0x80)
      break;
    bytes++;
  }
  bytes++;
  return value;
}

LIBREMIDI_STATIC uint32_t cmidi2_midi1_get_message_size(uint8_t* bytes, uint32_t length)
{
  switch (bytes[0])
  {
    case 0xF0: {
      uint8_t* start = bytes;
      uint8_t* end = bytes + length;
      for (bytes++; bytes < end; bytes++)
        if (*bytes == 0xF7)
          break;
      bytes++;
      return bytes - start;
    }
    case CMIDI2_SYSTEM_STATUS_MIDI_TIME_CODE:
    case CMIDI2_SYSTEM_STATUS_SONG_SELECT:
      return 2;
    case CMIDI2_SYSTEM_STATUS_SONG_POSITION:
      return 3;
    case CMIDI2_SYSTEM_STATUS_TUNE_REQUEST:
    case CMIDI2_SYSTEM_STATUS_TIMING_CLOCK:
    case CMIDI2_SYSTEM_STATUS_START:
    case CMIDI2_SYSTEM_STATUS_CONTINUE:
    case CMIDI2_SYSTEM_STATUS_STOP:
    case CMIDI2_SYSTEM_STATUS_ACTIVE_SENSING:
    case CMIDI2_SYSTEM_STATUS_RESET: // Use cmidi2_midi1_get_message_size_smf to handle meta-events
      return 1;
    default:
      switch (bytes[0] & 0xF0)
      {
        case 0xC0:
        case 0xD0:
          return 2;
        default:
          return 3;
      }
      break;
  }
  return 0;
}

LIBREMIDI_STATIC uint32_t cmidi2_midi1_get_message_size_live(uint8_t* bytes, uint32_t length)
{
  // 0xFF in live MIDI stream indicates reset. We handle it there.
  return cmidi2_midi1_get_message_size(bytes, length);
}

LIBREMIDI_STATIC uint32_t cmidi2_midi1_get_message_size_smf(uint8_t* bytes, uint32_t length)
{
  switch (bytes[0])
  {
    case 0xFF: // 0xFF in standard MIDI file indicates Meta-events, not RESET
    {
      uint32_t metaLength;
      uint8_t* start = bytes;
      uint8_t* end = bytes + length;

      bytes++;
      metaLength = cmidi2_midi1_get_7bit_encoded_int(bytes, end - bytes);
      bytes += metaLength + cmidi2_midi1_get_7bit_encoded_int_length(metaLength);

      return bytes - start;
    }
    default:
      return cmidi2_midi1_get_message_size(bytes, length);
  }
}

// MIDI1 to UMP Translator

/* Should we define some structs like this to support conversion from MIDI1 UMP to MIDI2 UMP and complicate the API?
   I'm not sure if there are enough need, and it's better to avoid another cmidi2_convert_midi1_to_ump() (200-ish LoC).

typedef struct cmidi2_midi1_sequence_midi1_bytes {
    // input MIDI1 messages or SMF events.
    uint8_t *midi1;
    // size of the input stream to process in bytes
    size_t midi1_num_bytes;
    // it is updated as per cmidi2_convert_midi1_messages_to_ump() proceeds the MIDI1 stream.
    size_t midi1_proceeded_bytes;
} cmidi2_midi1_sequence_midi1_bytes;

typedef struct cmidi2_midi1_sequence_umps {
    // MIDI1 UMP stream.
    cmidi2_ump* ump;
    // size (capacity) of the output stream in bytes
    size_t ump_num_bytes;
    // it is updated as per cmidi2_convert_midi1_messages_to_ump() proceeds the UMP stream.
    size_t ump_proceeded_bytes;
} cmidi2_midi1_sequence_ump;
*/

enum cmidi2_translator_endianness
{
  CMIDI2_TRANSLATOR_DEFAULT_ENDIAN,
  CMIDI2_TRANSLATOR_BIG_ENDIAN,
  CMIDI2_TRANSLATOR_LITTLE_ENDIAN
};

// Conversion from MIDI1 message bytes or SMF event list to MIDI2 UMP stream
//
// The conversion requires some preserved context e.g. RPN/NRPN/DTE
//  MSB/LSB, so we take this struct.
typedef struct cmidi2_midi_conversion_context
{
  // When it is true, it means the input MIDI1 stream contains delta time.
  // TODO: implement support for it
  bool is_midi1_smf;
  // Context tempo as in SMF specification defines.
  // If the MIDI1 stream is SMF and non-SMPTE delta time is used, then we have to calculate
  // the actual delta time length in SMPTE (then to JR timestamp).
  // TODO: implement support for it
  int32_t tempo;
  // input MIDI1 messages or SMF events.
  uint8_t* midi1;
  // size of the input stream to process in bytes
  size_t midi1_num_bytes;
  // it is updated as per cmidi2_convert_midi1_messages_to_ump() proceeds the MIDI1 stream.
  size_t midi1_proceeded_bytes;
  // output UMP stream.
  cmidi2_ump* ump;
  // size (capacity) of the output stream in bytes
  size_t ump_num_bytes;
  // it is updated as per cmidi2_convert_midi1_messages_to_ump() proceeds the UMP stream.
  size_t ump_proceeded_bytes;
  // DTE conversion target.
  // cmidi2_convert_midi1_messages_to_ump() will return *_INVALID_RPN or *_INVALID_NRPN
  // for such invalid sequences, and to report that correctly we need to preserve CC status.
  // They are initialized to 0x8080 that implies both bank (MSB) and index (LSB) are invalid (> 0x7F).
  // When they are assigned valid values, then (context_[n]rpn & 0x8080) will become 0.
  // When cmidi2_convert_midi1_messages_to_ump() encountered DTE LSB, they are consumed
  // and reset to the initial value (0x8080).
  int32_t context_rpn;
  int32_t context_nrpn;
  int32_t context_dte;
  // MIDI 2.0 Default Translation (UMP specification Appendix D.3) accepts only DTE LSB
  // as the conversion terminator, but cmidi2 allows DTE LSB to come first,
  // if this flag is enabled.
  bool allow_reordered_dte;
  // Bank Select CC is preserved for the next program change.
  // The initial value is 0x8080, same as RPN/NRPN/DTE.
  // After program change is set, it is reset to the initial value.
  uint32_t context_bank;
  // Group can be specified.
  uint8_t group;
  // Destination protocol: can be MIDI1 UMP or MIDI2 UMP.
  enum cmidi2_ci_protocol_values midi_protocol;
  // Sysex conversion can be done to sysex8
  bool use_sysex8;
  // Determine whether delta time is skipped or not (inserted like SMF) in the output MIDI1 stream.
  bool skip_delta_time;
  // UMP serialization endianness
  int32_t ump_serialization_endianness;
} cmidi2_midi_conversion_context;

enum cmidi2_midi_conversion_result
{
  CMIDI2_CONVERSION_RESULT_OK = 0,
  CMIDI2_CONVERSION_RESULT_OUT_OF_SPACE = 1,
  CMIDI2_CONVERSION_RESULT_INVALID_SYSEX = 0x10,
  CMIDI2_CONVERSION_RESULT_INVALID_DTE_SEQUENCE = 0x11,
  CMIDI2_CONVERSION_RESULT_INVALID_STATUS = 0x13,
  CMIDI2_CONVERSION_RESULT_INCOMPLETE_SYSEX7 = 0x20,
  CMIDI2_CONVERSION_RESULT_INVALID_INPUT = 0x40,
};

LIBREMIDI_STATIC void
cmidi2_midi_conversion_context_initialize(cmidi2_midi_conversion_context* context)
{
  context->is_midi1_smf = false;
  context->tempo = 500000;
  context->midi1 = NULL;
  context->midi1_num_bytes = 0;
  context->midi1_proceeded_bytes = 0;
  context->ump = NULL;
  context->ump_num_bytes = 0;
  context->ump_proceeded_bytes = 0;
  context->context_rpn = 0x8080;
  context->context_nrpn = 0x8080;
  context->context_dte = 0x8080;
  context->context_bank = 0x8080;
  context->allow_reordered_dte = false;
  context->group = 0;
  context->midi_protocol = CMIDI2_PROTOCOL_TYPE_MIDI2;
  context->use_sysex8 = false;
  context->skip_delta_time = false;
  context->ump_serialization_endianness = CMIDI2_TRANSLATOR_DEFAULT_ENDIAN;
}

typedef struct cmidi2_convert_sysex_context
{
  cmidi2_midi_conversion_context* conversion_context;
  size_t dst_offset;
} cmidi2_convert_sysex_context;

LIBREMIDI_STATIC void*
cmidi2_internal_convert_add_midi1_sysex7_ump_to_list(uint64_t data, void* context)
{
  cmidi2_convert_sysex_context* s7ctx = (cmidi2_convert_sysex_context*)context;
  s7ctx->conversion_context->ump[s7ctx->dst_offset] = data >> 32;
  s7ctx->conversion_context->ump[s7ctx->dst_offset + 1] = data & 0xFFFFFFFF;
  s7ctx->conversion_context->ump_proceeded_bytes += 2 * sizeof(cmidi2_ump);
  s7ctx->dst_offset += 2;
  return NULL;
}

LIBREMIDI_STATIC void* cmidi2_internal_convert_add_midi1_sysex8_ump_to_list(
    uint64_t data1, uint64_t data2, size_t index, void* context)
{
  (void)index;
  cmidi2_convert_sysex_context* s8ctx = (cmidi2_convert_sysex_context*)context;
  s8ctx->conversion_context->ump[s8ctx->dst_offset] = data1 >> 32;
  s8ctx->conversion_context->ump[s8ctx->dst_offset + 1] = data1 & 0xFFFFFFFF;
  s8ctx->conversion_context->ump[s8ctx->dst_offset + 2] = data2 >> 32;
  s8ctx->conversion_context->ump[s8ctx->dst_offset + 3] = data2 & 0xFFFFFFFF;
  s8ctx->conversion_context->ump_proceeded_bytes += 4 * sizeof(cmidi2_ump);
  s8ctx->dst_offset += 4;
  return NULL;
}

LIBREMIDI_STATIC uint64_t
cmidi2_internal_convert_midi1_dte_to_ump(cmidi2_midi_conversion_context* context, uint8_t channel)
{
  bool isRpn = (context->context_rpn & 0x8080) == 0;
  uint8_t msb = (isRpn ? context->context_rpn : context->context_nrpn) >> 8;
  uint8_t lsb = (isRpn ? context->context_rpn : context->context_nrpn) & 0xFF;
  int32_t data = (context->context_dte >> 8 << 25) + ((context->context_dte & 0x7F) << 18);
  // reset RPN/NRPN/DTE status to the initial values.
  context->context_rpn = 0x8080;
  context->context_nrpn = 0x8080;
  context->context_dte = 0x8080;
  return isRpn ? cmidi2_ump_midi2_rpn(context->group, channel, msb, lsb, data)
               : cmidi2_ump_midi2_nrpn(context->group, channel, msb, lsb, data);
}

LIBREMIDI_STATIC uint32_t cmidi2_internal_swap_endian(uint32_t v)
{
  return ((v & 0xFF) << 24) + (((v >> 8) & 0xFF) << 16) + (((v >> 16) & 0xFF) << 8)
         + ((v >> 24) & 0xFF);
}

/** converts MIDI1 bytestream which can contain deltaTime in SMF, to MIDI2 UMP stream.
 * The resulting stream is native endianness.
 */
static enum cmidi2_midi_conversion_result
cmidi2_convert_midi1_to_ump(cmidi2_midi_conversion_context* context)
{
  uint8_t* dst = (uint8_t*)context->ump;
  size_t sLen = context->midi1_num_bytes;
  size_t dLen = context->ump_num_bytes;
  uint8_t* sIdx = (uint8_t*)&context->midi1_proceeded_bytes;
  uint8_t* dIdx = (uint8_t*)&context->ump_proceeded_bytes;

  while (*sIdx < sLen)
  {
    // FIXME: implement deltaTime to JR Timestamp conversion.

    uint8_t status = context->midi1[*sIdx];
    if (status == 0xF0)
    {
      // sysex
      uint8_t* f7
          = (uint8_t*)memchr(context->midi1 + *sIdx, 0xF7, context->midi1_num_bytes - *sIdx);
      if (f7 == NULL)
      {
        return CMIDI2_CONVERSION_RESULT_INVALID_SYSEX; // error
      }
      size_t sysexSize = f7 - context->midi1 - *sIdx - 1; // excluding 0xF7
      size_t numPackets = context->use_sysex8 ? cmidi2_ump_sysex8_get_num_packets(sysexSize)
                                              : cmidi2_ump_sysex7_get_num_packets(sysexSize);
      if (dLen - *dIdx < numPackets)
        return CMIDI2_CONVERSION_RESULT_OUT_OF_SPACE;
      cmidi2_convert_sysex_context sysExCtx;
      sysExCtx.conversion_context = context;
      sysExCtx.dst_offset = *dIdx;
      if (context->use_sysex8)
      {
        // ignoring the return code as it never returns non-NULL... (size is already verified)
        cmidi2_ump_sysex8_process(
            context->group, context->midi1 + *sIdx, sysexSize, 0,
            cmidi2_internal_convert_add_midi1_sysex8_ump_to_list, &sysExCtx);
      }
      else
      {
        // ignoring the return code as it never returns non-NULL... (size is already verified)
        cmidi2_ump_sysex7_process(
            context->group, context->midi1 + *sIdx,
            cmidi2_internal_convert_add_midi1_sysex7_ump_to_list, &sysExCtx);
      }
      *sIdx += sysexSize + 2; // +1 for 0xF0 and 0xF7
    }
    else
    {
      // fixed sized message
      size_t remaining = sLen - *sIdx;
      size_t inputMidi1Len
          = context->is_midi1_smf
                ? cmidi2_midi1_get_message_size_smf(context->midi1 + *sIdx, remaining)
                : cmidi2_midi1_get_message_size_live(context->midi1 + *sIdx, remaining);
      if (inputMidi1Len > remaining)
        return CMIDI2_CONVERSION_RESULT_INVALID_INPUT;

      uint8_t byte2 = inputMidi1Len > 1 ? context->midi1[*sIdx + 1] : 0;
      uint8_t byte3 = inputMidi1Len > 2 ? context->midi1[*sIdx + 2] : 0;
      uint8_t channel = status & 0xF;
      if (context->midi_protocol == CMIDI2_PROTOCOL_TYPE_MIDI1)
      {
        // generate MIDI1 UMPs
        if (status > 0xF0)
        {
          dst[*dIdx] = cmidi2_ump_system_message(context->group, status, byte2, byte3);
          *sIdx += inputMidi1Len;
          *dIdx += 4;
        }
        else
        {
          dst[*dIdx]
              = cmidi2_ump_midi1_message(context->group, status & 0xF0, channel, byte2, byte3);
          *sIdx += inputMidi1Len;
          *dIdx += 4;
        }
      }
      else
      {
        // generate MIDI2 UMPs
        uint64_t m2;
        const int8_t NO_ATTRIBUTE_TYPE = 0;
        const int16_t NO_ATTRIBUTE_DATA = 0;
        bool bankValid, bankMsbValid, bankLsbValid;
        bool skipEmitUmp = false;
        int outputUmpLen = 0;
        switch (status & 0xF0)
        {
          case CMIDI2_STATUS_NOTE_OFF:
            m2 = cmidi2_ump_midi2_note_off(
                context->group, channel, byte2, NO_ATTRIBUTE_TYPE, byte3 << 9, NO_ATTRIBUTE_DATA);
            outputUmpLen = 2;
            break;
          case CMIDI2_STATUS_NOTE_ON:
            m2 = cmidi2_ump_midi2_note_on(
                context->group, channel, byte2, NO_ATTRIBUTE_TYPE, byte3 << 9, NO_ATTRIBUTE_DATA);
            outputUmpLen = 2;
            break;
          case CMIDI2_STATUS_PAF:
            m2 = cmidi2_ump_midi2_paf(context->group, channel, byte2, byte3 << 25);
            outputUmpLen = 2;
            break;
          case CMIDI2_STATUS_CC:
            switch (byte2)
            {
              case CMIDI2_CC_RPN_MSB:
                context->context_rpn = (context->context_rpn & 0xFF) | (byte3 << 8);
                skipEmitUmp = true;
                break;
              case CMIDI2_CC_RPN_LSB:
                context->context_rpn = (context->context_rpn & 0xFF00) | byte3;
                skipEmitUmp = true;
                break;
              case CMIDI2_CC_NRPN_MSB:
                context->context_nrpn = (context->context_nrpn & 0xFF) | byte3 << 8;
                skipEmitUmp = true;
                break;
              case CMIDI2_CC_NRPN_LSB:
                context->context_nrpn = (context->context_nrpn & 0xFF00) | byte3;
                skipEmitUmp = true;
                break;
              case CMIDI2_CC_DTE_MSB:
                context->context_dte = (context->context_dte & 0xFF) | (byte3 << 8);

                if (context->allow_reordered_dte && (context->context_dte & 0x8080) == 0)
                {
                  m2 = cmidi2_internal_convert_midi1_dte_to_ump(context, channel);
                  outputUmpLen = 2;
                }
                else
                  skipEmitUmp = true;

                break;
              case CMIDI2_CC_DTE_LSB:
                context->context_dte = (context->context_dte & 0xFF00) | byte3;

                if ((context->context_dte & 0x8000) && !context->allow_reordered_dte)
                  return CMIDI2_CONVERSION_RESULT_INVALID_DTE_SEQUENCE;
                if ((context->context_rpn & 0x8080) && (context->context_nrpn & 0x8080))
                  return CMIDI2_CONVERSION_RESULT_INVALID_DTE_SEQUENCE;
                m2 = cmidi2_internal_convert_midi1_dte_to_ump(context, channel);
                outputUmpLen = 2;

                break;
              case CMIDI2_CC_BANK_SELECT:
                context->context_bank = (context->context_bank & 0xFF) | (byte3 << 8);
                skipEmitUmp = true;
                break;
              case CMIDI2_CC_BANK_SELECT_LSB:
                context->context_bank = (context->context_bank & 0xFF00) | byte3;
                skipEmitUmp = true;
                break;
              default:
                m2 = cmidi2_ump_midi2_cc(context->group, channel, byte2, byte3 << 25);
                outputUmpLen = 2;
                break;
            }
            break;
          case CMIDI2_STATUS_PROGRAM:
            bankMsbValid = (context->context_bank & 0x8000) == 0;
            bankLsbValid = (context->context_bank & 0x80) == 0;
            bankValid = bankMsbValid || bankLsbValid;
            m2 = cmidi2_ump_midi2_program(
                context->group, channel,
                bankValid ? CMIDI2_PROGRAM_CHANGE_OPTION_BANK_VALID
                          : CMIDI2_PROGRAM_CHANGE_OPTION_NONE,
                byte2, bankMsbValid ? context->context_bank >> 8 : 0,
                bankLsbValid ? context->context_bank & 0x7F : 0);
            outputUmpLen = 2;
            context->context_bank = 0x8080;
            break;
          case CMIDI2_STATUS_CAF:
            m2 = cmidi2_ump_midi2_caf(context->group, channel, byte2 << 25);
            outputUmpLen = 2;
            break;
          case CMIDI2_STATUS_PITCH_BEND:
            // Note: Pitch Bend values in the MIDI 1.0 Protocol are presented as Little Endian.
            m2 = cmidi2_ump_midi2_pitch_bend_direct(
                context->group, channel, ((byte3 << 7) + byte2) << 18);
            outputUmpLen = 2;
            break;
          default:
            switch (status)
            {
              case CMIDI2_SYSTEM_STATUS_MIDI_TIME_CODE:
              case CMIDI2_SYSTEM_STATUS_SONG_SELECT:
                inputMidi1Len = 2;
                m2 = cmidi2_ump_system_message(context->group, status, byte2, byte3);
                outputUmpLen = 1;
                break;
              case CMIDI2_SYSTEM_STATUS_SONG_POSITION:
                inputMidi1Len = 3;
                m2 = cmidi2_ump_system_message(context->group, status, byte2, byte3);
                outputUmpLen = 1;
                break;
              case CMIDI2_SYSTEM_STATUS_TUNE_REQUEST:
              case CMIDI2_SYSTEM_STATUS_TIMING_CLOCK:
              case CMIDI2_SYSTEM_STATUS_START:
              case CMIDI2_SYSTEM_STATUS_CONTINUE:
              case CMIDI2_SYSTEM_STATUS_STOP:
              case CMIDI2_SYSTEM_STATUS_ACTIVE_SENSING:
              case CMIDI2_SYSTEM_STATUS_RESET:
                inputMidi1Len = 1;
                m2 = cmidi2_ump_system_message(context->group, status, 0, 0);
                outputUmpLen = 1;
                break;
              default:
                return CMIDI2_CONVERSION_RESULT_INVALID_STATUS;
            }
            break;
        }
        if (!skipEmitUmp)
        {
          switch (outputUmpLen)
          {
            case 1:
              *(uint32_t*)(dst + *dIdx) = m2;
              *dIdx += 4;
              break;
            case 2: {
              int platEndian = cmidi2_util_is_platform_little_endian()
                                   ? CMIDI2_TRANSLATOR_LITTLE_ENDIAN
                                   : CMIDI2_TRANSLATOR_BIG_ENDIAN;
              int actualEndian
                  = context->ump_serialization_endianness != CMIDI2_TRANSLATOR_DEFAULT_ENDIAN
                        ? context->ump_serialization_endianness
                        : platEndian;
              if (platEndian != actualEndian)
                m2 = (((uint64_t)cmidi2_internal_swap_endian(m2 >> 32)) << 32)
                     | cmidi2_internal_swap_endian(m2 & 0xFFFFFFFF);
              *(uint32_t*)(dst + *dIdx) = m2 >> 32;
              *dIdx += 4;
              *(uint32_t*)(dst + *dIdx) = m2 & 0xFFFFFFFF;
              *dIdx += 4;
              break;
            }
          }
        }
        *sIdx += inputMidi1Len;
      }
    }
  }
  // incomplete UMP sequence will be reported as CMIDI2_CONVERSION_RESULT_OUT_OF_SPACE,
  // so it is safe to judge that incomplete RPN/NRPN/DTE state at this state means invalid.
  if (context->context_rpn != 0x8080 || context->context_nrpn != 0x8080
      || context->context_dte != 0x8080)
    return CMIDI2_CONVERSION_RESULT_INVALID_DTE_SEQUENCE;

  return CMIDI2_CONVERSION_RESULT_OK;
}

// UMP to MIDI1 Translator

static int32_t cmidi2_internal_convert_jr_timestamp_to_timecode(
    int32_t deltaTime, cmidi2_midi_conversion_context* context)
{
  (void)context;
  // FIXME: implement
  return deltaTime;
}

static size_t cmidi2_internal_add_midi1_delta_time(
    uint8_t* dst, cmidi2_midi_conversion_context* context, int32_t deltaTime)
{
  if (!context || context->skip_delta_time)
    return 0;
  size_t* dIdx = (size_t*)&context->midi1_proceeded_bytes;
  int32_t len = cmidi2_midi1_get_7bit_encoded_int_length(deltaTime);
  cmidi2_midi1_write_7bit_encoded_int(dst, deltaTime);
  *dIdx += len;
  return len;
}

/// Convert one single UMP (without JR Timestamp) to MIDI 1.0 Message (without delta time)
/// It is a lengthy function, so it is recommended to wrap it in another non-inline function.
LIBREMIDI_STATIC size_t cmidi2_convert_single_ump_to_timed_midi1(
    uint8_t* dst, size_t maxBytes, cmidi2_ump* ump, int32_t deltaTime,
    cmidi2_midi_conversion_context* context, uint8_t* sysex7Buffer, size_t* sysex7BufferIndex)
{
  if (maxBytes < 1)
    return 0;

  size_t midiEventSize = 0;
  uint64_t sysex7U64;
  uint8_t sysex7NumBytesInUmp;

  uint8_t messageType = cmidi2_ump_get_message_type(ump);
  uint8_t statusCode = cmidi2_ump_get_status_code(ump); // may not apply, but won't break.

#define CMIDI2_INTERNAL_ADD_DELTA_TIME                                  \
  dst += cmidi2_internal_add_midi1_delta_time(dst, context, deltaTime); \
  dst[0] = statusCode | cmidi2_ump_get_channel(ump);

  switch (messageType)
  {
    case CMIDI2_MESSAGE_TYPE_SYSTEM:
      statusCode = cmidi2_ump_get_status_byte(ump);
      CMIDI2_INTERNAL_ADD_DELTA_TIME

      switch (statusCode)
      {
        case CMIDI2_SYSTEM_STATUS_SONG_POSITION:
          midiEventSize = 3;
          break;
        case CMIDI2_SYSTEM_STATUS_MIDI_TIME_CODE:
        case CMIDI2_SYSTEM_STATUS_SONG_SELECT:
          midiEventSize = 2;
          break;
        default:
          midiEventSize = 1;
          break;
      }
      if (maxBytes < midiEventSize)
        return 0;
      dst[0] = statusCode; // no channel filtering
      if (midiEventSize >= 2)
        dst[1] = cmidi2_ump_get_midi1_byte2(ump);
      if (midiEventSize >= 3)
        dst[2] = cmidi2_ump_get_midi1_byte3(ump);
      break;
    case CMIDI2_MESSAGE_TYPE_MIDI_1_CHANNEL:
      CMIDI2_INTERNAL_ADD_DELTA_TIME

      switch (statusCode)
      {
        case 0xC0:
        case 0xD0:
          midiEventSize = 2;
          if (maxBytes < midiEventSize)
            return 0;
          dst[1] = cmidi2_ump_get_midi1_byte2(ump);
          break;
        default:
          midiEventSize = 3;
          if (maxBytes < midiEventSize)
            return 0;
          dst[1] = cmidi2_ump_get_midi1_byte2(ump);
          dst[2] = cmidi2_ump_get_midi1_byte3(ump);
          break;
      }
      break;
    case CMIDI2_MESSAGE_TYPE_MIDI_2_CHANNEL:
      // FIXME: convert MIDI2 to MIDI1 as long as possible
      switch (statusCode)
      {
        case CMIDI2_STATUS_RPN:
          CMIDI2_INTERNAL_ADD_DELTA_TIME
          midiEventSize = 12;
          if (maxBytes < midiEventSize)
            return 0;
          dst[0] = cmidi2_ump_get_channel(ump) + CMIDI2_STATUS_CC;
          dst[1] = CMIDI2_CC_RPN_MSB;
          dst[2] = cmidi2_ump_get_midi2_rpn_msb(ump);
          dst[3] = dst[0]; // CC + channel
          dst[4] = CMIDI2_CC_RPN_LSB;
          dst[5] = cmidi2_ump_get_midi2_rpn_lsb(ump);
          dst[6] = dst[0]; // CC + channel
          dst[7] = CMIDI2_CC_DTE_MSB;
          dst[8] = (cmidi2_ump_get_midi2_rpn_data(ump) >> 25) & 0x7F;
          dst[9] = dst[0]; // CC + channel
          dst[10] = CMIDI2_CC_DTE_LSB;
          dst[11] = (cmidi2_ump_get_midi2_rpn_data(ump) >> 18) & 0x7F;
          break;
        case CMIDI2_STATUS_NRPN:
          CMIDI2_INTERNAL_ADD_DELTA_TIME
          midiEventSize = 12;
          if (maxBytes < midiEventSize)
            return 0;
          dst[0] = cmidi2_ump_get_channel(ump) + CMIDI2_STATUS_CC;
          dst[1] = CMIDI2_CC_NRPN_MSB;
          dst[2] = cmidi2_ump_get_midi2_nrpn_msb(ump);
          dst[3] = dst[0]; // CC + channel
          dst[4] = CMIDI2_CC_NRPN_LSB;
          dst[5] = cmidi2_ump_get_midi2_nrpn_lsb(ump);
          dst[6] = dst[0]; // CC + channel
          dst[7] = CMIDI2_CC_DTE_MSB;
          dst[8] = (cmidi2_ump_get_midi2_nrpn_data(ump) >> 25) & 0x7F;
          dst[9] = dst[0]; // CC + channel
          dst[10] = CMIDI2_CC_DTE_LSB;
          dst[11] = (cmidi2_ump_get_midi2_nrpn_data(ump) >> 18) & 0x7F;
          break;
        case CMIDI2_STATUS_NOTE_OFF:
        case CMIDI2_STATUS_NOTE_ON:
          CMIDI2_INTERNAL_ADD_DELTA_TIME
          midiEventSize = 3;
          if (maxBytes < midiEventSize)
            return 0;
          dst[1] = cmidi2_ump_get_midi2_note_note(ump);
          dst[2] = cmidi2_ump_get_midi2_note_velocity(ump) / 0x200;
          break;
        case CMIDI2_STATUS_PAF:
          CMIDI2_INTERNAL_ADD_DELTA_TIME
          midiEventSize = 3;
          if (maxBytes < midiEventSize)
            return 0;
          dst[1] = cmidi2_ump_get_midi2_paf_note(ump);
          dst[2] = cmidi2_ump_get_midi2_paf_data(ump) / 0x2000000;
          break;
        case CMIDI2_STATUS_CC:
          CMIDI2_INTERNAL_ADD_DELTA_TIME
          midiEventSize = 3;
          if (maxBytes < midiEventSize)
            return 0;
          dst[0] = statusCode | cmidi2_ump_get_channel(ump);
          dst[1] = cmidi2_ump_get_midi2_cc_index(ump);
          dst[2] = cmidi2_ump_get_midi2_cc_data(ump) / 0x2000000;
          break;
        case CMIDI2_STATUS_PROGRAM:
          CMIDI2_INTERNAL_ADD_DELTA_TIME
          dst[0] = statusCode | cmidi2_ump_get_channel(ump);
          if (cmidi2_ump_get_midi2_program_options(ump) & CMIDI2_PROGRAM_CHANGE_OPTION_BANK_VALID)
          {
            midiEventSize = 8;
            if (maxBytes < midiEventSize)
              return 0;
            dst[6] = dst[0]; // copy
            dst[7] = cmidi2_ump_get_midi2_program_program(ump);
            dst[0] = (dst[6] & 0xF) + CMIDI2_STATUS_CC;
            dst[1] = 0; // Bank MSB
            dst[2] = cmidi2_ump_get_midi2_program_bank_msb(ump);
            dst[3] = (dst[6] & 0xF) + CMIDI2_STATUS_CC;
            dst[4] = 32; // Bank LSB
            dst[5] = cmidi2_ump_get_midi2_program_bank_lsb(ump);
          }
          else
          {
            midiEventSize = 2;
            if (maxBytes < midiEventSize)
              return 0;
            dst[1] = cmidi2_ump_get_midi2_program_program(ump);
          }
          break;
        case CMIDI2_STATUS_CAF:
          CMIDI2_INTERNAL_ADD_DELTA_TIME
          midiEventSize = 2;
          if (maxBytes < midiEventSize)
            return 0;
          dst[1] = cmidi2_ump_get_midi2_caf_data(ump) / 0x2000000;
          break;
        case CMIDI2_STATUS_PITCH_BEND:
          CMIDI2_INTERNAL_ADD_DELTA_TIME
          midiEventSize = 3;
          if (maxBytes < midiEventSize)
            return 0;
          uint32_t pitchBendV1 = cmidi2_ump_get_midi2_pitch_bend_data(ump) / 0x40000;
          dst[1] = pitchBendV1 % 0x80;
          dst[2] = pitchBendV1 / 0x80;
          break;
          // skip for other status bytes; we cannot support them.
      }
      break;
    case CMIDI2_MESSAGE_TYPE_SYSEX7:
      if (sysex7Buffer)
      {
        // sysex7 buffer is processed at cmidi2_convert_ump_to_midi1().
        midiEventSize = 0;
        sysex7U64 = cmidi2_ump_read_uint64_bytes(ump);
        sysex7NumBytesInUmp = cmidi2_ump_get_sysex7_num_bytes(ump);
        for (size_t i = 0; i < sysex7NumBytesInUmp; i++)
          sysex7Buffer[*sysex7BufferIndex + i] = cmidi2_ump_get_byte_from_uint64(sysex7U64, 2 + i);
        *sysex7BufferIndex += sysex7NumBytesInUmp;
      }
      else
      {
        // minimal implementation for single-byte sysex7
        CMIDI2_INTERNAL_ADD_DELTA_TIME
        midiEventSize = 1 + cmidi2_ump_get_sysex7_num_bytes(ump);
        if (maxBytes < midiEventSize)
          return 0;

        sysex7U64 = cmidi2_ump_read_uint64_bytes(ump);
        for (size_t i = 0; i < midiEventSize - 1; i++)
          dst[i] = cmidi2_ump_get_byte_from_uint64(sysex7U64, 2 + i);
      }
      break;
    case CMIDI2_MESSAGE_TYPE_SYSEX8_MDS:
      // By the UMP specification they cannot be translated in Default Translation
      midiEventSize = 0;
      break;
  }
  return midiEventSize;
}

// Left for API backward compatibility.
LIBREMIDI_STATIC size_t
cmidi2_convert_single_ump_to_midi1(uint8_t* dst, size_t maxBytes, cmidi2_ump* ump)
{
  return cmidi2_convert_single_ump_to_timed_midi1(dst, maxBytes, ump, 0, NULL, NULL, 0);
}

#define IS_JR_TIMESTAMP(ump)                                       \
  (cmidi2_ump_get_message_type(ump) == CMIDI2_MESSAGE_TYPE_UTILITY \
   && cmidi2_ump_get_status_code(ump) == CMIDI2_UTILITY_STATUS_JR_TIMESTAMP)

static enum cmidi2_midi_conversion_result
cmidi2_convert_ump_to_midi1(cmidi2_midi_conversion_context* context)
{
  uint8_t* dst = (uint8_t*)context->midi1;
  size_t sLen = context->ump_num_bytes;
  size_t dLen = context->midi1_num_bytes;
  size_t* sIdx = (size_t*)&context->ump_proceeded_bytes;
  size_t* dIdx = (size_t*)&context->midi1_proceeded_bytes;
  uint8_t sysex7_buffer[1024];
  size_t sysex7_buffer_index = 0;

  while (*sIdx < sLen)
  {
    int32_t deltaTimeInJRTimestamp = 0;
    cmidi2_ump* ump;
    do
    {
      ump = (cmidi2_ump*)((uint8_t*)context->ump + *sIdx);
      if (IS_JR_TIMESTAMP(ump))
      {
        if (!context->skip_delta_time)
          deltaTimeInJRTimestamp += cmidi2_ump_get_jr_timestamp_timestamp(ump);
      }
      else
        break;
      *sIdx += 4; // 4 = sizeof JR Timestamp message
    } while (*sIdx < sLen);

    if (*sIdx >= sLen)
      break;

    int32_t deltaTime
        = cmidi2_internal_convert_jr_timestamp_to_timecode(deltaTimeInJRTimestamp, context);

    int32_t len = cmidi2_convert_single_ump_to_timed_midi1(
        dst + *dIdx, dLen - *dIdx, ump, deltaTime, context, sysex7_buffer, &sysex7_buffer_index);
    *dIdx += len;
    *sIdx += cmidi2_ump_get_num_bytes(*ump);

    if (cmidi2_ump_get_message_type(ump) == CMIDI2_MESSAGE_TYPE_SYSEX7)
    {
      switch (cmidi2_ump_get_status_code(ump))
      {
        case CMIDI2_SYSEX_END:
        case CMIDI2_SYSEX_IN_ONE_UMP:
          if (!context->skip_delta_time)
            cmidi2_internal_add_midi1_delta_time(dst + *dIdx, context, deltaTime);
          dst[*dIdx] = 0xF0;
          *dIdx += 1;
          memcpy(dst + *dIdx, sysex7_buffer, sysex7_buffer_index);
          *dIdx += sysex7_buffer_index;
          sysex7_buffer_index = 0;
          dst[*dIdx] = 0xF7;
          *dIdx += 1;
          break;
      }
    }
    deltaTime = 0;
  }

  return sysex7_buffer_index > 0 ? CMIDI2_CONVERSION_RESULT_INCOMPLETE_SYSEX7
                                 : CMIDI2_CONVERSION_RESULT_OK;
}

// UMP forge (like LV2 Atom Forge, but flat and simpler)

typedef struct cmidi2_ump_forge
{
  cmidi2_ump* ump;
  size_t capacity;
  size_t offset;
} cmidi2_ump_forge;

LIBREMIDI_STATIC void
cmidi2_ump_forge_init(cmidi2_ump_forge* forge, cmidi2_ump* buffer, size_t capacityInBytes)
{
  forge->ump = buffer;
  forge->capacity = capacityInBytes;
  forge->offset = 0;
}

LIBREMIDI_STATIC bool cmidi2_ump_forge_add_packet_32(cmidi2_ump_forge* forge, uint32_t ump)
{
  int size = 4;
  if (forge->offset + size > forge->capacity)
    return false;
  uint8_t* p = (uint8_t*)forge->ump + forge->offset;
  cmidi2_ump_write32((cmidi2_ump*)p, ump);
  forge->offset += size;
  return true;
}

LIBREMIDI_STATIC bool cmidi2_ump_forge_add_packet_64(cmidi2_ump_forge* forge, uint64_t ump)
{
  int size = 8;
  if (forge->offset + size > forge->capacity)
    return false;
  uint8_t* p = (uint8_t*)forge->ump + forge->offset;
  cmidi2_ump_write64((cmidi2_ump*)p, ump);
  forge->offset += size;
  return true;
}

LIBREMIDI_STATIC bool
cmidi2_ump_forge_add_packet_128(cmidi2_ump_forge* forge, uint64_t ump1, uint64_t ump2)
{
  int size = 16;
  if (forge->offset + size > forge->capacity)
    return false;
  uint8_t* p = ((uint8_t*)forge->ump) + forge->offset;
  cmidi2_ump_write128((cmidi2_ump*)p, ump1, ump2);
  forge->offset += size;
  return true;
}

LIBREMIDI_STATIC bool cmidi2_ump_forge_add_single_packet(cmidi2_ump_forge* forge, cmidi2_ump* ump)
{
  int size = cmidi2_ump_get_message_size_bytes(ump);
  if (forge->offset + size > forge->capacity)
    return false;
  memcpy((uint8_t*)forge->ump + forge->offset, ump, size);
  forge->offset += size;
  return true;
}

LIBREMIDI_STATIC bool
cmidi2_ump_forge_add_packets(cmidi2_ump_forge* forge, cmidi2_ump* ump, int32_t size)
{
  if (forge->offset + size > forge->capacity)
    return false;
  memcpy((uint8_t*)forge->ump + forge->offset, ump, size);
  forge->offset += size;
  return true;
}

// UMP sequence merger

LIBREMIDI_STATIC bool cmidi2_internal_ump_merge_sequence_write_delta_time(
    int32_t timestamp1, int32_t timestamp2, int32_t* lastTimestamp, void* dst, int32_t* dIdx,
    size_t dstSize)
{
  int32_t deltaTime = (timestamp1 <= timestamp2 ? timestamp1 : timestamp2) - *lastTimestamp;
  uint8_t jrTSSize = deltaTime / 0x10000 + (deltaTime % 0x10000 ? 1 : 0);
  if (*dIdx + jrTSSize * 4 > (int64_t)dstSize)
    return false;
  for (int32_t dt = deltaTime; dt > 0; dt -= 0x10000)
  {
    cmidi2_ump_write32(
        (cmidi2_ump*)((uint8_t*)dst + *dIdx),
        cmidi2_ump_jr_timestamp_direct(deltaTime > 0xFFFF ? 0xFFFF : deltaTime));
    *dIdx += 4;
  }
  *lastTimestamp += deltaTime;
  return true;
}

LIBREMIDI_STATIC size_t cmidi2_ump_merge_sequences(
    cmidi2_ump* dst, size_t dstCapacity, cmidi2_ump* seq1, size_t seq1Size, cmidi2_ump* seq2,
    size_t seq2Size)
{
  int32_t dIdx = 0, seq1Idx = 0, seq2Idx = 0;
  int32_t timestamp1 = 0, timestamp2 = 0, lastTimestamp = 0;
  while (true)
  {
    cmidi2_ump* s1 = (cmidi2_ump*)((uint8_t*)seq1 + seq1Idx);
    while (cmidi2_ump_get_message_type(s1) == CMIDI2_MESSAGE_TYPE_UTILITY
           && cmidi2_ump_get_status_code(s1) == CMIDI2_UTILITY_STATUS_JR_TIMESTAMP)
    {
      timestamp1 += cmidi2_ump_get_jr_timestamp_timestamp(s1);
      seq1Idx += 4;
      if (seq1Idx >= (int64_t)seq1Size)
        break;
      s1 = (cmidi2_ump*)((uint8_t*)seq1 + seq1Idx);
    }
    cmidi2_ump* s2 = (cmidi2_ump*)((uint8_t*)seq2 + seq2Idx);
    while (cmidi2_ump_get_message_type(s2) == CMIDI2_MESSAGE_TYPE_UTILITY
           && cmidi2_ump_get_status_code(s2) == CMIDI2_UTILITY_STATUS_JR_TIMESTAMP)
    {
      timestamp2 += cmidi2_ump_get_jr_timestamp_timestamp(s2);
      seq2Idx += 4;
      if (seq2Idx >= (int64_t)seq2Size)
        break;
      s2 = (cmidi2_ump*)((uint8_t*)seq2 + seq2Idx);
    }
    if (seq1Idx >= (int64_t)seq1Size || seq2Idx >= (int64_t)seq2Size)
      break;
    if (!cmidi2_internal_ump_merge_sequence_write_delta_time(
            timestamp1, timestamp2, &lastTimestamp, dst, &dIdx, dstCapacity))
      break;

    if (timestamp1 <= timestamp2)
    {
      cmidi2_ump* sp = (cmidi2_ump*)((uint8_t*)seq1 + seq1Idx);
      uint8_t size = cmidi2_ump_get_message_size_bytes(sp);
      if (size == 0)
        return dIdx; // invalid bytes
      memcpy((uint8_t*)dst + dIdx, sp, size);
      seq1Idx += size;
      dIdx += size;
    }
    else
    {
      cmidi2_ump* sp = (cmidi2_ump*)((uint8_t*)seq2 + seq2Idx);
      uint8_t size = cmidi2_ump_get_message_size_bytes(sp);
      if (size == 0)
        return dIdx; // invalid bytes
      memcpy((uint8_t*)dst + dIdx, sp, size);
      seq2Idx += size;
      dIdx += size;
    }
  }
  if (!cmidi2_internal_ump_merge_sequence_write_delta_time(
          timestamp1, timestamp2, &lastTimestamp, dst, &dIdx, dstCapacity))
    return dIdx;
  if (seq1Idx < (int64_t)seq1Size && dIdx + (int64_t)seq1Size - seq1Idx < (int64_t)dstCapacity)
  {
    cmidi2_ump* sp = (cmidi2_ump*)((uint8_t*)seq1 + seq1Idx);
    memcpy((uint8_t*)dst + dIdx, sp, seq1Size - seq1Idx);
    dIdx += seq1Size - seq1Idx;
  }
  if (seq2Idx < (int64_t)seq2Size && dIdx + (int64_t)seq2Size - seq2Idx < (int64_t)dstCapacity)
  {
    cmidi2_ump* sp = (cmidi2_ump*)((uint8_t*)seq2 + seq2Idx);
    memcpy((uint8_t*)dst + dIdx, sp, seq2Size - seq2Idx);
    dIdx += seq2Size - seq2Idx;
  }
  return dIdx;
}

#ifdef __cplusplus
}
#endif

#endif /* CMIDI2_H_INCLUDED */
