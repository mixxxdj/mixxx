#pragma once

#include <libremidi/backends/linux/dylib_loader.hpp>

#include <alsa/asoundlib.h> // IWYU pragma: export

#if defined(SND_LIB_VERSION)
  #if __has_include(<alsa/rawmidi.h>) && SND_LIB_VERSION >= ((1 << 16) | (2 << 8) | 6)
    #define LIBREMIDI_ALSA_HAS_RAMWIDI 1
    #define LIBREMIDI_ALSA_HAS_RAWMIDI_TREAD 1
  #endif

  #if __has_include(<alsa/ump.h>) && SND_LIB_VERSION >= ((1 << 16) | (2 << 8) | 10)
    #define LIBREMIDI_ALSA_HAS_UMP 1
  #endif

  #if __has_include(<alsa/ump.h>) && SND_LIB_VERSION >= ((1 << 16) | (2 << 8) | 14)
    #define LIBREMIDI_ALSA_HAS_UMP_SEQ_EVENTS 1
  #endif
#endif

NAMESPACE_LIBREMIDI
{

struct libasound
{
  // Useful one-liner:
  // nm -A * | grep ' snd_' | grep -v '@' | cut -f 2 -d 'U' | sort | uniq  | sed 's/ snd_//' | sed 's/_/, /' | awk ' { print "LIBREMIDI_SYMBOL_DEF(snd_"$1 " " $2 ");" }'

  explicit libasound()
      : library{"libasound.so.2"}
  {
    if (!library)
    {
      available = false;
      return;
    }

    strerror = library.symbol<decltype(&::snd_strerror)>("snd_strerror");
    if (!strerror)
      available = false;
  }

  static const libasound& instance()
  {
    static const libasound self;
    return self;
  }

  dylib_loader library;
  decltype(&::snd_strerror) strerror{};
  bool available{true};

  struct card_t
  {
    explicit card_t(const dylib_loader& library)
    {
      if (!library)
      {
        available = false;
        return;
      }

      LIBREMIDI_SYMBOL_INIT(snd_card, get_name)
      LIBREMIDI_SYMBOL_INIT(snd_card, next)
    }
    bool available{true};

    LIBREMIDI_SYMBOL_DEF(snd_card, get_name)
    LIBREMIDI_SYMBOL_DEF(snd_card, next)
  } card{library};

  struct ctl_t
  {
    explicit ctl_t(const dylib_loader& library)
        : rawmidi{library}
#if LIBREMIDI_ALSA_HAS_UMP
        , ump{library}
#endif
    {
      if (!library)
      {
        available = false;
        return;
      }

      LIBREMIDI_SYMBOL_INIT(snd_ctl, close)
      LIBREMIDI_SYMBOL_INIT(snd_ctl, open)
    }
    bool available{true};

    LIBREMIDI_SYMBOL_DEF(snd_ctl, close)
    LIBREMIDI_SYMBOL_DEF(snd_ctl, open)

    struct rawmidi_t
    {
      explicit rawmidi_t(const dylib_loader& library)
      {
        if (!library)
        {
          available = false;
          return;
        }

        LIBREMIDI_SYMBOL_INIT(snd_ctl_rawmidi, info)
        LIBREMIDI_SYMBOL_INIT(snd_ctl_rawmidi, next_device)
      }
      bool available{true};
      LIBREMIDI_SYMBOL_DEF(snd_ctl_rawmidi, info)
      LIBREMIDI_SYMBOL_DEF(snd_ctl_rawmidi, next_device)
    } rawmidi;

#if LIBREMIDI_ALSA_HAS_UMP
    struct ump_t
    {
      explicit ump_t(const dylib_loader& library)
      {
        if (!library)
        {
          available = false;
          return;
        }

        LIBREMIDI_SYMBOL_INIT(snd_ctl_ump, block_info)
        LIBREMIDI_SYMBOL_INIT(snd_ctl_ump, endpoint_info)
        LIBREMIDI_SYMBOL_INIT(snd_ctl_ump, next_device)
      }
      bool available{true};
      LIBREMIDI_SYMBOL_DEF(snd_ctl_ump, block_info)
      LIBREMIDI_SYMBOL_DEF(snd_ctl_ump, endpoint_info)
      LIBREMIDI_SYMBOL_DEF(snd_ctl_ump, next_device)
    } ump;
#endif
  } ctl{library};

  struct midi_t
  {
    explicit midi_t(const dylib_loader& library)
    {
      if (!library)
      {
        available = false;
        return;
      }

      LIBREMIDI_SYMBOL_INIT(snd_midi, event_decode)
      LIBREMIDI_SYMBOL_INIT(snd_midi, event_encode)
      LIBREMIDI_SYMBOL_INIT(snd_midi, event_free)
      LIBREMIDI_SYMBOL_INIT(snd_midi, event_init)
      LIBREMIDI_SYMBOL_INIT(snd_midi, event_new)
      LIBREMIDI_SYMBOL_INIT(snd_midi, event_no_status)
      LIBREMIDI_SYMBOL_INIT(snd_midi, event_resize_buffer)
    }

    bool available{true};
    LIBREMIDI_SYMBOL_DEF(snd_midi, event_decode)
    LIBREMIDI_SYMBOL_DEF(snd_midi, event_encode)
    LIBREMIDI_SYMBOL_DEF(snd_midi, event_free)
    LIBREMIDI_SYMBOL_DEF(snd_midi, event_init)
    LIBREMIDI_SYMBOL_DEF(snd_midi, event_new)
    LIBREMIDI_SYMBOL_DEF(snd_midi, event_no_status)
    LIBREMIDI_SYMBOL_DEF(snd_midi, event_resize_buffer)
  } midi{library};

#if LIBREMIDI_ALSA_HAS_RAMWIDI
  struct rawmidi_t
  {
    explicit rawmidi_t(const dylib_loader& library)
    {
      if (!library)
      {
        available = false;
        return;
      }
      LIBREMIDI_SYMBOL_INIT(snd_rawmidi, close)
      LIBREMIDI_SYMBOL_INIT(snd_rawmidi, info_get_name)
      LIBREMIDI_SYMBOL_INIT(snd_rawmidi, info_get_subdevice_name)
      LIBREMIDI_SYMBOL_INIT(snd_rawmidi, info_get_subdevices_count)
      LIBREMIDI_SYMBOL_INIT(snd_rawmidi, info_set_device)
      LIBREMIDI_SYMBOL_INIT(snd_rawmidi, info_set_stream)
      LIBREMIDI_SYMBOL_INIT(snd_rawmidi, info_set_subdevice)
      LIBREMIDI_SYMBOL_INIT(snd_rawmidi, info_sizeof)
      LIBREMIDI_SYMBOL_INIT(snd_rawmidi, open)
      LIBREMIDI_SYMBOL_INIT(snd_rawmidi, params)
      LIBREMIDI_SYMBOL_INIT(snd_rawmidi, params_current)
      LIBREMIDI_SYMBOL_INIT(snd_rawmidi, params_get_buffer_size)
      LIBREMIDI_SYMBOL_INIT(snd_rawmidi, params_set_clock_type)
      LIBREMIDI_SYMBOL_INIT(snd_rawmidi, params_set_no_active_sensing)
      LIBREMIDI_SYMBOL_INIT(snd_rawmidi, params_set_read_mode)
      LIBREMIDI_SYMBOL_INIT(snd_rawmidi, params_sizeof)
      LIBREMIDI_SYMBOL_INIT(snd_rawmidi, poll_descriptors)
      LIBREMIDI_SYMBOL_INIT(snd_rawmidi, poll_descriptors_count)
      LIBREMIDI_SYMBOL_INIT(snd_rawmidi, poll_descriptors_revents)
      LIBREMIDI_SYMBOL_INIT(snd_rawmidi, read)
      LIBREMIDI_SYMBOL_INIT(snd_rawmidi, status)
      LIBREMIDI_SYMBOL_INIT(snd_rawmidi, status_get_avail)
      LIBREMIDI_SYMBOL_INIT(snd_rawmidi, status_sizeof)
      LIBREMIDI_SYMBOL_INIT(snd_rawmidi, tread)
      LIBREMIDI_SYMBOL_INIT(snd_rawmidi, write)
    }

    bool available{true};
    LIBREMIDI_SYMBOL_DEF(snd_rawmidi, close)
    LIBREMIDI_SYMBOL_DEF(snd_rawmidi, info_get_name)
    LIBREMIDI_SYMBOL_DEF(snd_rawmidi, info_get_subdevice_name)
    LIBREMIDI_SYMBOL_DEF(snd_rawmidi, info_get_subdevices_count)
    LIBREMIDI_SYMBOL_DEF(snd_rawmidi, info_set_device)
    LIBREMIDI_SYMBOL_DEF(snd_rawmidi, info_set_stream)
    LIBREMIDI_SYMBOL_DEF(snd_rawmidi, info_set_subdevice)
    LIBREMIDI_SYMBOL_DEF(snd_rawmidi, info_sizeof)
    LIBREMIDI_SYMBOL_DEF(snd_rawmidi, open)
    LIBREMIDI_SYMBOL_DEF(snd_rawmidi, params)
    LIBREMIDI_SYMBOL_DEF(snd_rawmidi, params_current)
    LIBREMIDI_SYMBOL_DEF(snd_rawmidi, params_get_buffer_size)
    LIBREMIDI_SYMBOL_DEF(snd_rawmidi, params_set_clock_type)
    LIBREMIDI_SYMBOL_DEF(snd_rawmidi, params_set_no_active_sensing)
    LIBREMIDI_SYMBOL_DEF(snd_rawmidi, params_set_read_mode)
    LIBREMIDI_SYMBOL_DEF(snd_rawmidi, params_sizeof)
    LIBREMIDI_SYMBOL_DEF(snd_rawmidi, poll_descriptors)
    LIBREMIDI_SYMBOL_DEF(snd_rawmidi, poll_descriptors_count)
    LIBREMIDI_SYMBOL_DEF(snd_rawmidi, poll_descriptors_revents)
    LIBREMIDI_SYMBOL_DEF(snd_rawmidi, read)
    LIBREMIDI_SYMBOL_DEF(snd_rawmidi, status)
    LIBREMIDI_SYMBOL_DEF(snd_rawmidi, status_get_avail)
    LIBREMIDI_SYMBOL_DEF(snd_rawmidi, status_sizeof)
    LIBREMIDI_SYMBOL_DEF(snd_rawmidi, tread)
    LIBREMIDI_SYMBOL_DEF(snd_rawmidi, write)
  } rawmidi{library};
#endif

  struct seq_t
  {
    explicit seq_t(const dylib_loader& library)
#if LIBREMIDI_ALSA_HAS_UMP
        : ump{library}
#endif
    {
      if (!library)
      {
        available = false;
        return;
      }

      LIBREMIDI_SYMBOL_INIT(snd_seq, alloc_queue)
      LIBREMIDI_SYMBOL_INIT(snd_seq, client_id)
      LIBREMIDI_SYMBOL_INIT(snd_seq, client_info_get_client)
      LIBREMIDI_SYMBOL_INIT(snd_seq, client_info_get_name)
      LIBREMIDI_SYMBOL_INIT(snd_seq, client_info_get_card)
      LIBREMIDI_SYMBOL_INIT(snd_seq, client_info_set_client)
      LIBREMIDI_SYMBOL_INIT(snd_seq, client_info_sizeof)
      LIBREMIDI_SYMBOL_INIT(snd_seq, close)
      LIBREMIDI_SYMBOL_INIT(snd_seq, connect_from)
      LIBREMIDI_SYMBOL_INIT(snd_seq, control_queue)
      LIBREMIDI_SYMBOL_INIT(snd_seq, create_port)
      LIBREMIDI_SYMBOL_INIT(snd_seq, delete_port)
      LIBREMIDI_SYMBOL_INIT(snd_seq, drain_output)
      LIBREMIDI_SYMBOL_INIT(snd_seq, event_input)
      LIBREMIDI_SYMBOL_INIT(snd_seq, event_input_pending)
      LIBREMIDI_SYMBOL_INIT(snd_seq, event_output)
      LIBREMIDI_SYMBOL_INIT(snd_seq, free_event)
      LIBREMIDI_SYMBOL_INIT(snd_seq, free_queue)
      LIBREMIDI_SYMBOL_INIT(snd_seq, get_any_client_info)
      LIBREMIDI_SYMBOL_INIT(snd_seq, get_any_port_info)
      LIBREMIDI_SYMBOL_INIT(snd_seq, get_port_info)
      LIBREMIDI_SYMBOL_INIT(snd_seq, open)
      LIBREMIDI_SYMBOL_INIT(snd_seq, poll_descriptors)
      LIBREMIDI_SYMBOL_INIT(snd_seq, poll_descriptors_count)
      LIBREMIDI_SYMBOL_INIT(snd_seq, port_info_get_addr)
      LIBREMIDI_SYMBOL_INIT(snd_seq, port_info_get_capability)
      LIBREMIDI_SYMBOL_INIT(snd_seq, port_info_get_name)
      LIBREMIDI_SYMBOL_INIT(snd_seq, port_info_get_port)
      LIBREMIDI_SYMBOL_INIT(snd_seq, port_info_get_type)
      LIBREMIDI_SYMBOL_INIT(snd_seq, port_info_set_capability)
      LIBREMIDI_SYMBOL_INIT(snd_seq, port_info_set_client)
      LIBREMIDI_SYMBOL_INIT(snd_seq, port_info_set_midi_channels)
      LIBREMIDI_SYMBOL_INIT(snd_seq, port_info_set_name)
      LIBREMIDI_SYMBOL_INIT(snd_seq, port_info_set_port)
      LIBREMIDI_SYMBOL_INIT(snd_seq, port_info_set_timestamping)
      LIBREMIDI_SYMBOL_INIT(snd_seq, port_info_set_timestamp_queue)
      LIBREMIDI_SYMBOL_INIT(snd_seq, port_info_set_timestamp_real)
      LIBREMIDI_SYMBOL_INIT(snd_seq, port_info_set_type)
      LIBREMIDI_SYMBOL_INIT(snd_seq, port_info_sizeof)
      LIBREMIDI_SYMBOL_INIT(snd_seq, port_subscribe_free)
      LIBREMIDI_SYMBOL_INIT(snd_seq, port_subscribe_malloc)
      LIBREMIDI_SYMBOL_INIT(snd_seq, port_subscribe_sizeof)
      LIBREMIDI_SYMBOL_INIT(snd_seq, port_subscribe_set_dest)
      LIBREMIDI_SYMBOL_INIT(snd_seq, port_subscribe_set_sender)
      LIBREMIDI_SYMBOL_INIT(snd_seq, port_subscribe_set_time_real)
      LIBREMIDI_SYMBOL_INIT(snd_seq, port_subscribe_set_time_update)
      LIBREMIDI_SYMBOL_INIT(snd_seq, query_next_client)
      LIBREMIDI_SYMBOL_INIT(snd_seq, query_next_port)
      LIBREMIDI_SYMBOL_INIT(snd_seq, queue_tempo_set_ppq)
      LIBREMIDI_SYMBOL_INIT(snd_seq, queue_tempo_set_tempo)
      LIBREMIDI_SYMBOL_INIT(snd_seq, queue_tempo_sizeof)
      LIBREMIDI_SYMBOL_INIT(snd_seq, set_client_name)
      LIBREMIDI_SYMBOL_INIT(snd_seq, set_port_info)
      LIBREMIDI_SYMBOL_INIT(snd_seq, set_queue_tempo)
      LIBREMIDI_SYMBOL_INIT(snd_seq, subscribe_port)
      LIBREMIDI_SYMBOL_INIT(snd_seq, unsubscribe_port)

#if LIBREMIDI_ALSA_HAS_UMP
      LIBREMIDI_SYMBOL_INIT(snd_seq, set_client_midi_version)
      LIBREMIDI_SYMBOL_INIT(snd_seq, get_ump_endpoint_info)
      LIBREMIDI_SYMBOL_INIT(snd_seq, get_ump_block_info)
#endif
    }

    bool available{true};
    LIBREMIDI_SYMBOL_DEF(snd_seq, alloc_queue)
    LIBREMIDI_SYMBOL_DEF(snd_seq, client_id)
    LIBREMIDI_SYMBOL_DEF(snd_seq, client_info_get_client)
    LIBREMIDI_SYMBOL_DEF(snd_seq, client_info_get_name)
    LIBREMIDI_SYMBOL_DEF(snd_seq, client_info_get_card)
    LIBREMIDI_SYMBOL_DEF(snd_seq, client_info_set_client)
    LIBREMIDI_SYMBOL_DEF(snd_seq, client_info_sizeof)
    LIBREMIDI_SYMBOL_DEF(snd_seq, close)
    LIBREMIDI_SYMBOL_DEF(snd_seq, connect_from)
    LIBREMIDI_SYMBOL_DEF(snd_seq, control_queue)
    LIBREMIDI_SYMBOL_DEF(snd_seq, create_port)
    LIBREMIDI_SYMBOL_DEF(snd_seq, delete_port)
    LIBREMIDI_SYMBOL_DEF(snd_seq, drain_output)
    LIBREMIDI_SYMBOL_DEF(snd_seq, event_input)
    LIBREMIDI_SYMBOL_DEF(snd_seq, event_input_pending)
    LIBREMIDI_SYMBOL_DEF(snd_seq, event_output)
    LIBREMIDI_SYMBOL_DEF(snd_seq, free_event)
    LIBREMIDI_SYMBOL_DEF(snd_seq, free_queue)
    LIBREMIDI_SYMBOL_DEF(snd_seq, get_any_client_info)
    LIBREMIDI_SYMBOL_DEF(snd_seq, get_any_port_info)
    LIBREMIDI_SYMBOL_DEF(snd_seq, get_port_info)
    LIBREMIDI_SYMBOL_DEF(snd_seq, open)
    LIBREMIDI_SYMBOL_DEF(snd_seq, poll_descriptors)
    LIBREMIDI_SYMBOL_DEF(snd_seq, poll_descriptors_count)
    LIBREMIDI_SYMBOL_DEF(snd_seq, port_info_get_addr)
    LIBREMIDI_SYMBOL_DEF(snd_seq, port_info_get_capability)
    LIBREMIDI_SYMBOL_DEF(snd_seq, port_info_get_name)
    LIBREMIDI_SYMBOL_DEF(snd_seq, port_info_get_port)
    LIBREMIDI_SYMBOL_DEF(snd_seq, port_info_get_type)
    LIBREMIDI_SYMBOL_DEF(snd_seq, port_info_set_capability)
    LIBREMIDI_SYMBOL_DEF(snd_seq, port_info_set_client)
    LIBREMIDI_SYMBOL_DEF(snd_seq, port_info_set_midi_channels)
    LIBREMIDI_SYMBOL_DEF(snd_seq, port_info_set_name)
    LIBREMIDI_SYMBOL_DEF(snd_seq, port_info_set_port)
    LIBREMIDI_SYMBOL_DEF(snd_seq, port_info_set_timestamping)
    LIBREMIDI_SYMBOL_DEF(snd_seq, port_info_set_timestamp_queue)
    LIBREMIDI_SYMBOL_DEF(snd_seq, port_info_set_timestamp_real)
    LIBREMIDI_SYMBOL_DEF(snd_seq, port_info_set_type)
    LIBREMIDI_SYMBOL_DEF(snd_seq, port_info_sizeof)
    LIBREMIDI_SYMBOL_DEF(snd_seq, port_subscribe_free)
    LIBREMIDI_SYMBOL_DEF(snd_seq, port_subscribe_malloc)
    LIBREMIDI_SYMBOL_DEF(snd_seq, port_subscribe_sizeof)
    LIBREMIDI_SYMBOL_DEF(snd_seq, port_subscribe_set_dest)
    LIBREMIDI_SYMBOL_DEF(snd_seq, port_subscribe_set_sender)
    LIBREMIDI_SYMBOL_DEF(snd_seq, port_subscribe_set_time_real)
    LIBREMIDI_SYMBOL_DEF(snd_seq, port_subscribe_set_time_update)
    LIBREMIDI_SYMBOL_DEF(snd_seq, query_next_client)
    LIBREMIDI_SYMBOL_DEF(snd_seq, query_next_port)
    LIBREMIDI_SYMBOL_DEF(snd_seq, queue_tempo_set_ppq)
    LIBREMIDI_SYMBOL_DEF(snd_seq, queue_tempo_set_tempo)
    LIBREMIDI_SYMBOL_DEF(snd_seq, queue_tempo_sizeof)

    LIBREMIDI_SYMBOL_DEF(snd_seq, set_client_name)
    LIBREMIDI_SYMBOL_DEF(snd_seq, set_port_info)
    LIBREMIDI_SYMBOL_DEF(snd_seq, set_queue_tempo)
    LIBREMIDI_SYMBOL_DEF(snd_seq, subscribe_port)
    LIBREMIDI_SYMBOL_DEF(snd_seq, unsubscribe_port)

#if LIBREMIDI_ALSA_HAS_UMP
    LIBREMIDI_SYMBOL_DEF(snd_seq, set_client_midi_version)
    LIBREMIDI_SYMBOL_DEF(snd_seq, get_ump_endpoint_info)
    LIBREMIDI_SYMBOL_DEF(snd_seq, get_ump_block_info)
#endif

#if LIBREMIDI_ALSA_HAS_UMP
    struct ump_t
    {
      explicit ump_t(const dylib_loader& library)
      {
        if (!library)
        {
          available = false;
          return;
        }
        LIBREMIDI_SYMBOL_INIT(snd_seq_ump, event_input)
        LIBREMIDI_SYMBOL_INIT(snd_seq_ump, event_output)
        LIBREMIDI_SYMBOL_INIT(snd_seq_ump, event_output_direct)
      }

      bool available{true};

      LIBREMIDI_SYMBOL_DEF(snd_seq_ump, event_input)
      LIBREMIDI_SYMBOL_DEF(snd_seq_ump, event_output)
      LIBREMIDI_SYMBOL_DEF(snd_seq_ump, event_output_direct)

    } ump;
#endif
  } seq{library};

#if LIBREMIDI_ALSA_HAS_UMP
  struct ump_t
  {
    explicit ump_t(const dylib_loader& library)
    {
      if (!library)
      {
        available = false;
        return;
      }

      // Core UMP functions
      LIBREMIDI_SYMBOL_INIT(snd_ump, open)
      LIBREMIDI_SYMBOL_INIT(snd_ump, close)
      LIBREMIDI_SYMBOL_INIT(snd_ump, rawmidi)
      LIBREMIDI_SYMBOL_INIT(snd_ump, rawmidi_params)
      LIBREMIDI_SYMBOL_INIT(snd_ump, rawmidi_params_current)
      LIBREMIDI_SYMBOL_INIT(snd_ump, read)
      LIBREMIDI_SYMBOL_INIT(snd_ump, tread)
      LIBREMIDI_SYMBOL_INIT(snd_ump, write)
      LIBREMIDI_SYMBOL_INIT(snd_ump, poll_descriptors)
      LIBREMIDI_SYMBOL_INIT(snd_ump, poll_descriptors_count)
      LIBREMIDI_SYMBOL_INIT(snd_ump, poll_descriptors_revents)

      // Endpoint info functions
      LIBREMIDI_SYMBOL_INIT(snd_ump, endpoint_info)
      LIBREMIDI_SYMBOL_INIT(snd_ump, endpoint_info_sizeof)
      LIBREMIDI_SYMBOL_INIT(snd_ump, endpoint_info_get_card)
      LIBREMIDI_SYMBOL_INIT(snd_ump, endpoint_info_get_device)
      LIBREMIDI_SYMBOL_INIT(snd_ump, endpoint_info_get_flags)
      LIBREMIDI_SYMBOL_INIT(snd_ump, endpoint_info_get_protocol_caps)
      LIBREMIDI_SYMBOL_INIT(snd_ump, endpoint_info_get_protocol)
      LIBREMIDI_SYMBOL_INIT(snd_ump, endpoint_info_get_num_blocks)
      LIBREMIDI_SYMBOL_INIT(snd_ump, endpoint_info_get_version)
      LIBREMIDI_SYMBOL_INIT(snd_ump, endpoint_info_get_manufacturer_id)
      LIBREMIDI_SYMBOL_INIT(snd_ump, endpoint_info_get_family_id)
      LIBREMIDI_SYMBOL_INIT(snd_ump, endpoint_info_get_model_id)
      LIBREMIDI_SYMBOL_INIT(snd_ump, endpoint_info_get_sw_revision)
      LIBREMIDI_SYMBOL_INIT(snd_ump, endpoint_info_get_name)
      LIBREMIDI_SYMBOL_INIT(snd_ump, endpoint_info_get_product_id)

      // Block info functions
      LIBREMIDI_SYMBOL_INIT(snd_ump, block_info)
      LIBREMIDI_SYMBOL_INIT(snd_ump, block_info_sizeof)
      LIBREMIDI_SYMBOL_INIT(snd_ump, block_info_set_block_id)
      LIBREMIDI_SYMBOL_INIT(snd_ump, block_info_get_block_id)
      LIBREMIDI_SYMBOL_INIT(snd_ump, block_info_get_active)
      LIBREMIDI_SYMBOL_INIT(snd_ump, block_info_get_flags)
      LIBREMIDI_SYMBOL_INIT(snd_ump, block_info_get_direction)
      LIBREMIDI_SYMBOL_INIT(snd_ump, block_info_get_first_group)
      LIBREMIDI_SYMBOL_INIT(snd_ump, block_info_get_num_groups)
      LIBREMIDI_SYMBOL_INIT(snd_ump, block_info_get_midi_ci_version)
      LIBREMIDI_SYMBOL_INIT(snd_ump, block_info_get_sysex8_streams)
      LIBREMIDI_SYMBOL_INIT(snd_ump, block_info_get_ui_hint)
      LIBREMIDI_SYMBOL_INIT(snd_ump, block_info_get_name)
    }

    bool available{true};

    // Core UMP functions
    LIBREMIDI_SYMBOL_DEF(snd_ump, open)
    LIBREMIDI_SYMBOL_DEF(snd_ump, close)
    LIBREMIDI_SYMBOL_DEF(snd_ump, rawmidi)
    LIBREMIDI_SYMBOL_DEF(snd_ump, rawmidi_params)
    LIBREMIDI_SYMBOL_DEF(snd_ump, rawmidi_params_current)
    LIBREMIDI_SYMBOL_DEF(snd_ump, read)
    LIBREMIDI_SYMBOL_DEF(snd_ump, tread)
    LIBREMIDI_SYMBOL_DEF(snd_ump, write)
    LIBREMIDI_SYMBOL_DEF(snd_ump, poll_descriptors)
    LIBREMIDI_SYMBOL_DEF(snd_ump, poll_descriptors_count)
    LIBREMIDI_SYMBOL_DEF(snd_ump, poll_descriptors_revents)

    // Endpoint info functions
    LIBREMIDI_SYMBOL_DEF(snd_ump, endpoint_info)
    LIBREMIDI_SYMBOL_DEF(snd_ump, endpoint_info_sizeof)
    LIBREMIDI_SYMBOL_DEF(snd_ump, endpoint_info_get_card)
    LIBREMIDI_SYMBOL_DEF(snd_ump, endpoint_info_get_device)
    LIBREMIDI_SYMBOL_DEF(snd_ump, endpoint_info_get_flags)
    LIBREMIDI_SYMBOL_DEF(snd_ump, endpoint_info_get_protocol_caps)
    LIBREMIDI_SYMBOL_DEF(snd_ump, endpoint_info_get_protocol)
    LIBREMIDI_SYMBOL_DEF(snd_ump, endpoint_info_get_num_blocks)
    LIBREMIDI_SYMBOL_DEF(snd_ump, endpoint_info_get_version)
    LIBREMIDI_SYMBOL_DEF(snd_ump, endpoint_info_get_manufacturer_id)
    LIBREMIDI_SYMBOL_DEF(snd_ump, endpoint_info_get_family_id)
    LIBREMIDI_SYMBOL_DEF(snd_ump, endpoint_info_get_model_id)
    LIBREMIDI_SYMBOL_DEF(snd_ump, endpoint_info_get_sw_revision)
    LIBREMIDI_SYMBOL_DEF(snd_ump, endpoint_info_get_name)
    LIBREMIDI_SYMBOL_DEF(snd_ump, endpoint_info_get_product_id)

    // Block info functions
    LIBREMIDI_SYMBOL_DEF(snd_ump, block_info)
    LIBREMIDI_SYMBOL_DEF(snd_ump, block_info_sizeof)
    LIBREMIDI_SYMBOL_DEF(snd_ump, block_info_set_block_id)
    LIBREMIDI_SYMBOL_DEF(snd_ump, block_info_get_block_id)
    LIBREMIDI_SYMBOL_DEF(snd_ump, block_info_get_active)
    LIBREMIDI_SYMBOL_DEF(snd_ump, block_info_get_flags)
    LIBREMIDI_SYMBOL_DEF(snd_ump, block_info_get_direction)
    LIBREMIDI_SYMBOL_DEF(snd_ump, block_info_get_first_group)
    LIBREMIDI_SYMBOL_DEF(snd_ump, block_info_get_num_groups)
    LIBREMIDI_SYMBOL_DEF(snd_ump, block_info_get_midi_ci_version)
    LIBREMIDI_SYMBOL_DEF(snd_ump, block_info_get_sysex8_streams)
    LIBREMIDI_SYMBOL_DEF(snd_ump, block_info_get_ui_hint)
    LIBREMIDI_SYMBOL_DEF(snd_ump, block_info_get_name)
  } ump{library};
#endif
};

#undef snd_dylib_alloca
#define snd_dylib_alloca(ptr, access, type)                                \
  {                                                                        \
    *ptr = (snd_##access##_##type##_t*)alloca(snd.access.type##_sizeof()); \
    memset(*ptr, 0, snd.access.type##_sizeof());                           \
  }
#define snd_dylib_alloca2(ptr, access1, access2, type)                                         \
  {                                                                                            \
    *ptr = (snd_##access1##_access2##_##type##_t*)alloca(snd.access1.access2.type##_sizeof()); \
    memset(*ptr, 0, snd.access1.access2.type##_sizeof());                                      \
  }

#undef snd_rawmidi_info_alloca
#define snd_rawmidi_info_alloca(ptr) snd_dylib_alloca(ptr, rawmidi, info)
#undef snd_rawmidi_params_alloca
#define snd_rawmidi_params_alloca(ptr) snd_dylib_alloca(ptr, rawmidi, params)
#undef snd_rawmidi_status_alloca
#define snd_rawmidi_status_alloca(ptr) snd_dylib_alloca(ptr, rawmidi, status)

#undef snd_seq_client_info_alloca
#define snd_seq_client_info_alloca(ptr) snd_dylib_alloca(ptr, seq, client_info)
#undef snd_seq_port_info_alloca
#define snd_seq_port_info_alloca(ptr) snd_dylib_alloca(ptr, seq, port_info)
#undef snd_seq_port_subscribe_alloca
#define snd_seq_port_subscribe_alloca(ptr) snd_dylib_alloca(ptr, seq, port_subscribe)
#undef snd_seq_queue_tempo_alloca
#define snd_seq_queue_tempo_alloca(ptr) snd_dylib_alloca(ptr, seq, queue_tempo)

#if LIBREMIDI_ALSA_HAS_UMP
  #undef snd_ump_block_info_alloca
  #define snd_ump_block_info_alloca(ptr) snd_dylib_alloca(ptr, ump, block_info)
  #undef snd_ump_endpoint_info_alloca
  #define snd_ump_endpoint_info_alloca(ptr) snd_dylib_alloca(ptr, ump, endpoint_info)
#endif
}
