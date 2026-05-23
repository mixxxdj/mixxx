#pragma once
#include <libremidi/api-c.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#if defined(LIBREMIDI_EXPORTS)
  #if defined(_MSC_VER)
    #define LIBREMIDI_EXPORT __declspec(dllexport)
  #elif defined(__GNUC__) || defined(__clang__)
    #define LIBREMIDI_EXPORT __attribute__((visibility("default")))
  #endif
#else
  #define LIBREMIDI_EXPORT
#endif

#if __cplusplus
extern "C" {
#endif
typedef unsigned char libremidi_midi1_symbol;
typedef libremidi_midi1_symbol* libremidi_midi1_message;

typedef uint32_t libremidi_midi2_symbol;
typedef libremidi_midi2_symbol* libremidi_midi2_message;

typedef int64_t libremidi_timestamp;

typedef struct libremidi_midi_in_port libremidi_midi_in_port;
typedef struct libremidi_midi_out_port libremidi_midi_out_port;
typedef struct libremidi_midi_in_handle libremidi_midi_in_handle;
typedef struct libremidi_midi_out_handle libremidi_midi_out_handle;
typedef struct libremidi_midi_observer_handle libremidi_midi_observer_handle;

typedef struct libremidi_api_configuration libremidi_api_configuration;

enum libremidi_timestamp_mode
{
  NoTimestamp,

  Relative,
  Absolute,
  SystemMonotonic,
  AudioFrame,
  Custom
};

typedef struct libremidi_api_configuration
{
  enum libremidi_api api;
  enum
  {
    Observer,
    Input,
    Output
  } configuration_type;

  void* data;
} libremidi_api_configuration;

typedef struct libremidi_observer_configuration
{
  struct
  {
    void* context;
    void (*callback)(void* ctx, const char* error, size_t error_len, const void* source_location);
  } on_error;
  struct
  {
    void* context;
    void (*callback)(void* ctx, const char* error, size_t error_len, const void* source_location);
  } on_warning;

  struct
  {
    void* context;
    void (*callback)(void* ctx, const libremidi_midi_in_port*);
  } input_added;
  struct
  {
    void* context;
    void (*callback)(void* ctx, const libremidi_midi_in_port*);
  } input_removed;
  struct
  {
    void* context;
    void (*callback)(void* ctx, const libremidi_midi_out_port*);
  } output_added;
  struct
  {
    void* context;
    void (*callback)(void* ctx, const libremidi_midi_out_port*);
  } output_removed;

  bool track_hardware;
  bool track_virtual;
  bool track_any;
  bool notify_in_constructor;
} libremidi_observer_configuration;

typedef struct libremidi_midi1_callback
{
  void* context;
  void (*callback)(void* ctx, libremidi_timestamp, const libremidi_midi1_symbol*, size_t len);
} libremidi_midi1_callback;

typedef struct libremidi_midi2_callback
{
  void* context;
  void (*callback)(void* ctx, libremidi_timestamp, const libremidi_midi2_symbol*, size_t len);
} libremidi_midi2_callback;

typedef struct libremidi_midi_configuration
{
  // Indicates the kind of callback requested, e.g. set MIDI1_RAW if you use the on_midi1_raw_data
  enum
  {
    MIDI1 = (1 << 1),
    MIDI1_RAW = (1 << 2),
    MIDI2 = (1 << 3),
    MIDI2_RAW = (1 << 4)
  } version;

  union
  {
    libremidi_midi_in_port* in_port;
    libremidi_midi_out_port* out_port;
  };

  union
  {
    libremidi_midi1_callback on_midi1_message;
    libremidi_midi1_callback on_midi1_raw_data;
    libremidi_midi2_callback on_midi2_message;
    libremidi_midi2_callback on_midi2_raw_data;
  };

  struct
  {
    void* context;
    libremidi_timestamp (*callback)(void* ctx, libremidi_timestamp);
  } get_timestamp;

  struct
  {
    void* context;
    void (*callback)(void* ctx, const char* error, size_t error_len, const void* source_location);
  } on_error;
  struct
  {
    void* context;
    void (*callback)(void* ctx, const char* error, size_t error_len, const void* source_location);
  } on_warning;

  const char* port_name;
  bool virtual_port;

  bool ignore_sysex;
  bool ignore_timing;
  bool ignore_sensing;

  enum libremidi_timestamp_mode timestamps;
} libremidi_midi_configuration;

/// API utilities
LIBREMIDI_EXPORT
const char* libremidi_get_version(void);

LIBREMIDI_EXPORT
void libremidi_midi1_available_apis(void* ctx, void (*)(void*, libremidi_api));
LIBREMIDI_EXPORT
void libremidi_midi2_available_apis(void* ctx, void (*)(void*, libremidi_api));
LIBREMIDI_EXPORT
const char* libremidi_api_identifier(libremidi_api);
LIBREMIDI_EXPORT
const char* libremidi_api_display_name(libremidi_api);
LIBREMIDI_EXPORT
libremidi_api libremidi_get_compiled_api_by_identifier(const char*);

/// Create configurations
LIBREMIDI_EXPORT
int libremidi_midi_api_configuration_init(libremidi_api_configuration*);

LIBREMIDI_EXPORT
int libremidi_midi_observer_configuration_init(libremidi_observer_configuration*);

LIBREMIDI_EXPORT
int libremidi_midi_configuration_init(libremidi_midi_configuration*);

/// Read information about port objects
LIBREMIDI_EXPORT
int libremidi_midi_in_port_clone(const libremidi_midi_in_port* port, libremidi_midi_in_port** dst);

LIBREMIDI_EXPORT
int libremidi_midi_in_port_free(libremidi_midi_in_port* port);

LIBREMIDI_EXPORT
int libremidi_midi_in_port_name(
    const libremidi_midi_in_port* port, const char** name, size_t* len);

LIBREMIDI_EXPORT
int libremidi_midi_in_port_handle(
    const libremidi_midi_in_port* port, uint64_t* handle);

LIBREMIDI_EXPORT
int libremidi_midi_out_port_clone(
    const libremidi_midi_out_port* port, libremidi_midi_out_port** dst);

LIBREMIDI_EXPORT
int libremidi_midi_out_port_free(libremidi_midi_out_port* port);

LIBREMIDI_EXPORT
int libremidi_midi_out_port_name(
    const libremidi_midi_out_port* port, const char** name, size_t* len);

LIBREMIDI_EXPORT
int libremidi_midi_out_port_handle(
    const libremidi_midi_out_port* port, uint64_t* handle);

/// Observer API
LIBREMIDI_EXPORT
int libremidi_midi_observer_new(
    const libremidi_observer_configuration*, libremidi_api_configuration*,
    libremidi_midi_observer_handle**);

LIBREMIDI_EXPORT
int libremidi_midi_observer_enumerate_input_ports(
    libremidi_midi_observer_handle*, void* context,
    void (*)(void* ctx, const libremidi_midi_in_port*));

LIBREMIDI_EXPORT
int libremidi_midi_observer_enumerate_output_ports(
    libremidi_midi_observer_handle*, void* context,
    void (*)(void* ctx, const libremidi_midi_out_port*));

LIBREMIDI_EXPORT
int libremidi_midi_observer_free(libremidi_midi_observer_handle*);

/// MIDI input API (read MIDI messages)
LIBREMIDI_EXPORT
int libremidi_midi_in_new(
    const libremidi_midi_configuration*, const libremidi_api_configuration*,
    libremidi_midi_in_handle**);

LIBREMIDI_EXPORT
int libremidi_midi_in_is_connected(const libremidi_midi_in_handle*);

LIBREMIDI_EXPORT
libremidi_timestamp libremidi_midi_in_absolute_timestamp(libremidi_midi_in_handle*);

LIBREMIDI_EXPORT
int libremidi_midi_in_free(libremidi_midi_in_handle*);

/// MIDI output API (send MIDI messages)
LIBREMIDI_EXPORT
int libremidi_midi_out_new(
    const libremidi_midi_configuration*, const libremidi_api_configuration*,
    libremidi_midi_out_handle**);

LIBREMIDI_EXPORT
int libremidi_midi_out_is_connected(const libremidi_midi_out_handle*);

LIBREMIDI_EXPORT
int libremidi_midi_out_send_message(
    libremidi_midi_out_handle*, const libremidi_midi1_symbol*, size_t);

LIBREMIDI_EXPORT
int libremidi_midi_out_send_ump(libremidi_midi_out_handle*, const libremidi_midi2_symbol*, size_t);

LIBREMIDI_EXPORT
int libremidi_midi_out_schedule_message(
    libremidi_midi_out_handle*, int64_t ts, const libremidi_midi1_symbol*, size_t);

LIBREMIDI_EXPORT
int libremidi_midi_out_schedule_ump(
    libremidi_midi_out_handle*, int64_t ts, const libremidi_midi2_symbol*, size_t);

LIBREMIDI_EXPORT
int libremidi_midi_out_free(libremidi_midi_out_handle*);

#if __cplusplus
}
#endif
