#include "util/battery/batterylinux.h"

// Contains 'signals' so we undefine Qt's 'signals'.
#undef signals
#include <upower.h>

#include <QtDebug>

BatteryLinux::BatteryLinux(QObject* pParent)
    : Battery(pParent) {
}

BatteryLinux::~BatteryLinux() {
}

void BatteryLinux::read() {
    m_iMinutesLeft = Battery::TIME_UNKNOWN;
    m_dPercentage = 0.0;
    m_chargingState = Battery::UNKNOWN;

    // NOTE(rryan): It would be nice if we could create the client
    // once. However, while testing this up_client_get_devices(client) returned
    // an empty list when I tried to re-use the UpClient instance.
    UpClient* client = up_client_new();
    VERIFY_OR_DEBUG_ASSERT(client) {
      return;
    }

#if !UP_CHECK_VERSION(0, 9, 99)
    // Re-enumerate in case a device is added.
    up_client_enumerate_devices_sync(client, NULL, NULL);
#endif

#if UP_CHECK_VERSION(0, 99, 8)
    GPtrArray* devices = up_client_get_devices2(client);
    VERIFY_OR_DEBUG_ASSERT(devices) {
      return;
    }
#else
    // This deprecated function doesn't set the free function for
    // the array elements so we need to do it!
    // https://bugs.freedesktop.org/show_bug.cgi?id=106740
    // https://gitlab.freedesktop.org/upower/upower/issues/14
    GPtrArray* devices = up_client_get_devices(client);
    VERIFY_OR_DEBUG_ASSERT(devices) {
      return;
    }
    g_ptr_array_set_free_func(devices, (GDestroyNotify) g_object_unref);
#endif

    for (guint i = 0; i < devices->len; ++i) {
      gpointer device = g_ptr_array_index(devices, i);
      VERIFY_OR_DEBUG_ASSERT(device) {
        continue;
      }

      gboolean online;
      gdouble percentage;
      guint state;
      guint kind;
      gint64 timeToEmpty;
      gint64 timeToFull;
      g_object_get(G_OBJECT(device),
                   "percentage", &percentage,
                   "online", &online,
                   "state", &state,
                   "kind", &kind,
                   "time-to-empty", &timeToEmpty,
                   "time-to-full", &timeToFull,
                   NULL);

      // qDebug() << "BatteryLinux::read()"
      //          << "online" << online
      //          << "percentage" << percentage
      //          << "state" << state
      //          << "kind" << kind
      //          << "timeToEmpty" << timeToEmpty
      //          << "timeToFull" << timeToFull;

      if (kind == UP_DEVICE_KIND_BATTERY) {
        if (state == UP_DEVICE_STATE_CHARGING) {
          m_chargingState = CHARGING;
        } else if (state == UP_DEVICE_STATE_DISCHARGING) {
          m_chargingState = DISCHARGING;
        } else if (state == UP_DEVICE_STATE_FULLY_CHARGED) {
          m_chargingState = CHARGED;
        } else {
          m_chargingState = UNKNOWN;
        }

        m_dPercentage = percentage;

        // upower tells us the remaining time in seconds (0 if unknown)
        if (m_chargingState == CHARGING && timeToFull > 0) {
          m_iMinutesLeft = timeToFull / 60;
        } else if (m_chargingState == DISCHARGING && timeToEmpty > 0) {
          m_iMinutesLeft = timeToEmpty / 60;
        }
        break;
      }
    }

    g_ptr_array_free(devices, TRUE);
    g_object_unref(client);
}
