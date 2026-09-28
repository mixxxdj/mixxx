#pragma once

#include <mutex>

namespace mixxx {
namespace hid {

/// hidapi is not thread-safe on every platform. On macOS in particular
/// hid_enumerate(), hid_open(), hid_close() and hid_exit() share a global
/// IOHIDManager, and calling them concurrently corrupts the heap and aborts
/// (crash in IOHIDManagerSetDeviceMatching -> malloc_zone_error).
///
/// Every hidapi call that may touch this shared state must be serialized
/// through this mutex.
inline std::mutex& hidapiMutex() {
    static std::mutex mutex;
    return mutex;
}

} // namespace hid
} // namespace mixxx
