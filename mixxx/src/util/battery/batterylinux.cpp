#include "util/battery/batterylinux.h"

// Contains 'signals' so we undefine Qt's 'signals'.
#undef signals
#include <upower.h>

#include "moc_batterylinux.cpp"
#include "util/assert.h"

BatteryLinux::BatteryLinux(QObject* pParent)
    : Battery(pParent),
      m_client(up_client_new()) {
}

BatteryLinux::~BatteryLinux() {
    g_object_unref(static_cast<UpClient*>(m_client));
}

void BatteryLinux::read() {
    VERIFY_OR_DEBUG_ASSERT(static_cast<UpClient*>(m_client)) {
        return;
    }
    m_iMinutesLeft = Battery::TIME_UNKNOWN;
    m_dPercentage = 0.0;
    m_chargingState = Battery::UNKNOWN;

    UpDevice *device = up_client_get_display_device(static_cast<UpClient*>(m_client));
    if (!device) {
        return;
    }

    guint kind = UP_DEVICE_KIND_UNKNOWN;
    gboolean isPresent = false;
    guint state = UP_DEVICE_STATE_UNKNOWN;
    gdouble percentage = 0.0;
    gint64 timeToEmpty = 0;
    gint64 timeToFull = 0;
    g_object_get(G_OBJECT(device),
                 "kind", &kind,
                 "is-present", &isPresent,
                 "state", &state,
                 "percentage", &percentage,
                 "time-to-empty", &timeToEmpty,
                 "time-to-full", &timeToFull,
                 NULL);
    g_clear_object(&device);

    if (!isPresent || kind != UP_DEVICE_KIND_BATTERY) {
        return;
    }

    if (state == UP_DEVICE_STATE_CHARGING) {
        m_chargingState = CHARGING;
    } else if (state == UP_DEVICE_STATE_DISCHARGING) {
        m_chargingState = DISCHARGING;
    } else if (state == UP_DEVICE_STATE_FULLY_CHARGED) {
        m_chargingState = CHARGED;
    }

    m_dPercentage = percentage;

    // upower tells us the remaining time in seconds (0 if unknown)
    if (m_chargingState == CHARGING && timeToFull > 0) {
        m_iMinutesLeft = timeToFull / 60;
    } else if (m_chargingState == DISCHARGING && timeToEmpty > 0) {
        m_iMinutesLeft = timeToEmpty / 60;
    }
}
