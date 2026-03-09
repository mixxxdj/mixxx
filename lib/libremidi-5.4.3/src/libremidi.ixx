module;
// Platform config
#if defined(_WIN32)
#define NOMINMAX 1
#define WIN32_LEAN_AND_MEAN 1
#endif

#if __has_include(<boost/container/small_vector.hpp>) && !defined(LIBREMIDI_NO_BOOST)
  #if LIBREMIDI_SLIM_MESSAGE > 0
    #include <boost/container/static_vector.hpp>
  #else
    #include <boost/container/small_vector.hpp>
  #endif
#endif

#if __has_include(<boost/variant2.hpp>) && !defined(LIBREMIDI_NO_BOOST_VARIANT2)
  #include <boost/variant2.hpp>
#endif

// Platform headers
#if defined(LIBREMIDI_ALSA)
  #include <alsa/asoundlib.h>

  #if __has_include(<libudev.h>)
    #include <libudev.h>
  #endif
#endif

#if defined(LIBREMIDI_ANDROID)
  #include <amidi/AMidi.h>
  #include <android/log.h>
  #include <jni.h>
#endif

#if defined(LIBREMIDI_COREMIDI)
  #include <CoreAudio/CoreAudioTypes.h>
  #include <CoreAudio/HostTime.h>
  #include <CoreMIDI/CoreMIDI.h>
  #include <CoreServices/CoreServices.h>

  #if TARGET_OS_IPHONE
    #include <mach/mach_time.h>
  #endif
#endif

#if defined(LIBREMIDI_EMSCRIPTEN)
  #include <emscripten.h>
#endif

#if __has_include(<weakjack/weak_libjack.h>)
  #include <weakjack/weak_libjack.h>
#elif __has_include(<weak_libjack.h>)
  #include <weak_libjack.h>
#elif __has_include(<jack/jack.h>)
  #include <jack/jack.h>
  #include <jack/midiport.h>
  #include <jack/ringbuffer.h>
#endif

#if defined(LIBREMIDI_NETWORK)
  #include <boost/asio/io_context.hpp>
  #include <boost/asio/ip/udp.hpp>
  #include <boost/container/small_vector.hpp>
  #include <boost/container/static_vector.hpp>
  #include <boost/endian.hpp>
  #include <boost/lockfree/spsc_queue.hpp>
#endif

#if defined(LIBREMIDI_PIPEWIRE)
  #include <pipewire/filter.h>
  #include <pipewire/pipewire.h>

  #include <spa/control/control.h>
  #include <spa/param/audio/format-utils.h>
  #include <spa/param/props.h>
  #include <spa/utils/defs.h>
  #include <spa/utils/result.h>
  #if __has_include(<readerwriterqueue.h>)
    #include <readerwriterqueue.h>
  #endif
#endif

#if defined(_WIN32)
  #include <windows.h>
  #include <guiddef.h>
  #include <unknwn.h>
  #if defined(LIBREMIDI_WINMM)
    #include <mmsystem.h>
  #endif

  #if defined(LIBREMIDI_WINUWP)
    #include <winrt/Windows.Devices.Enumeration.h>
    #include <winrt/Windows.Devices.Midi.h>
    #include <winrt/Windows.Foundation.Collections.h>
    #include <winrt/Windows.Foundation.h>
    #include <winrt/Windows.Storage.Streams.h>
  #endif

  #if defined(LIBREMIDI_WINMIDI)
    #include <winrt/Microsoft.Windows.Devices.Midi2.h>
    #include <WindowsMidiServicesAppSdkComExtensions.h>
    #include <init/Microsoft.Windows.Devices.Midi2.Initialization.hpp>
  #endif
#endif

#if !defined(_WIN32)
  #include <dlfcn.h>
  #include <poll.h>
  #include <pthread.h>
  #include <unistd.h>

  #if __has_include(<sys/eventfd.h>)
    #include <sys/eventfd.h>
  #endif

  #if __has_include(<sys/timerfd.h>)
    #include <sys/timerfd.h>
  #endif

  #if __has_include(<sys/time.h>)
    #include <sys/time.h>
  #endif
#endif


#if LIBREMIDI_NI_MIDI2_COMPAT
  #include <midi/sysex.h>
  #include <midi/universal_packet.h>
#endif

#include <cmath>

#include <algorithm>
#include <any>
#include <array>
#include <atomic>
#include <bit>
#include <cassert>
#include <cctype>
#include <cerrno>
#include <chrono>
#include <cinttypes>
#include <compare>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <functional>
#include <iosfwd>
#include <iostream>
#include <limits>
#include <map>
#include <memory>
#include <mutex>
#include <new>
#include <optional>
#include <ostream>
#include <semaphore>

#if __has_include(<source_location>) && (__cpp_lib_source_location >= 201907L)
#include <source_location>
#endif

#include <span>
#include <stdexcept>

#if __has_include(<stop_token>) && __cpp_lib_jthread >= 201911L
#include <stop_token>
#endif

#include <string>
#include <string_view>
#include <system_error>
#include <thread>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <variant>
#include <vector>
#include <version>

#include <memory.h>

// External includes
export module libremidi;

#define LIBREMIDI_MODULE_BUILD 1
// #define LIBREMIDI_HEADER_ONLY 1

// Internal libremidi headers

#include <libremidi/config.hpp>

#undef LIBREMIDI_STATIC
#undef LIBREMIDI_STATIC_IMPLEMENTATION
#undef LIBREMIDI_STATIC_INLINE_IMPLEMENTATION
#undef LIBREMIDI_INLINE_IMPLEMENTATION
#undef LIBREMIDI_ANONYMOUS_NAMESPACE
#undef LIBREMIDI_INLINE
#define LIBREMIDI_STATIC
#define LIBREMIDI_STATIC_IMPLEMENTATION
#define LIBREMIDI_STATIC_INLINE_IMPLEMENTATION
#define LIBREMIDI_INLINE_IMPLEMENTATION
#define LIBREMIDI_ANONYMOUS_NAMESPACE detail
#define LIBREMIDI_INLINE
#include <libremidi/api-c.h>
#include <libremidi/api.hpp>
#include <libremidi/backends.hpp>
#include <libremidi/cmidi2.hpp>
#include <libremidi/configurations.hpp>
#include <libremidi/defaults.hpp>
#include <libremidi/detail/conversion.hpp>
#include <libremidi/detail/memory.hpp>
#include <libremidi/detail/midi_api.hpp>
#include <libremidi/detail/midi_in.hpp>
#include <libremidi/detail/midi_out.hpp>
#include <libremidi/detail/midi_stream_decoder.hpp>
#include <libremidi/detail/observer.hpp>
#include <libremidi/detail/semaphore.hpp>
#include <libremidi/detail/ump_stream.hpp>
#include <libremidi/error.hpp>
#include <libremidi/error_handler.hpp>
#include <libremidi/input_configuration.hpp>
#include <libremidi/libremidi-c.h>
#include <libremidi/libremidi.hpp>
#include <libremidi/message.hpp>
#include <libremidi/observer_configuration.hpp>
#include <libremidi/output_configuration.hpp>
#include <libremidi/reader.hpp>
#include <libremidi/shared_context.hpp>
#include <libremidi/system_error2.hpp>
#include <libremidi/types.hpp>
#include <libremidi/ump.hpp>
#include <libremidi/writer.hpp>

#if defined(__clang__) || defined(_MSC_VER)
module :private;
#endif

#undef NAMESPACE_LIBREMIDI
#define NAMESPACE_LIBREMIDI namespace libremidi
#include <libremidi/libremidi.cpp>
#include <libremidi/midi_in.cpp>
#include <libremidi/midi_out.cpp>
#include <libremidi/observer.cpp>
#include <libremidi/reader.cpp>
#include <libremidi/writer.cpp>
// #include <libremidi/libremidi-c.cpp>
