#pragma once

#include <vector>

struct libusb_context;

namespace mixxx {
namespace android {

enum class DeviceType : int {
    HID,
    BULK
};

struct DeviceIface {
    intptr_t fd;
    int num;
    DeviceType type;
};

extern std::vector<DeviceIface> devices;
extern libusb_context* libusb_ctx;
extern bool devicesReady;
extern std::mutex m_deviceMutex;
extern std::condition_variable m_deviceReadyWaitCond;

bool wait_for_ready();

} // namespace android
} // namespace mixxx
