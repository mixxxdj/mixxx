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
    m_iMinutesLeft = 0;
    m_dPercentage = 0.0;
    m_chargingState = Battery::UNKNOWN;

    // NOTE(rryan): It would be nice if we could create the client
    // once. However, while testing this up_client_get_devices(client) returned
    // an empty list when I tried to re-use the UpClient instance.
    UpClient* client = up_client_new();
    if (client == nullptr) {
      return;
    }

#if !UP_CHECK_VERSION(0, 9, 99)
    // Re-enumerate in case a device is added.
    up_client_enumerate_devices_sync(client, NULL, NULL);
#endif

    GPtrArray* devices = up_client_get_devices(client);
    if (devices == nullptr) {
      return;
    }

    for (guint i = 0; i < devices->len; ++i) {
      gpointer device = g_ptr_array_index(devices, i);
      if (device == nullptr) {
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
        if (m_chargingState == CHARGING) {
          m_iMinutesLeft = timeToFull / 60;
        } else if (m_chargingState == DISCHARGING) {
          m_iMinutesLeft = timeToEmpty / 60;
        }
        break;
      }
    }

    g_ptr_array_free(devices, TRUE);
    g_object_unref(client);
}
