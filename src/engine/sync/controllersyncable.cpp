#include "engine/sync/controllersyncable.h"

ControllerSyncable::ControllerSyncable(const QString& group)
        : Syncable(),
          m_group(group),
          m_syncMode(SYNC_INVALID) {
    m_pSyncLeaderEnabled.reset(new ControlPushButton(ConfigKey(m_group, "sync_master")));
    m_pSyncLeaderEnabled->setButtonMode(ControlPushButton::TOGGLE);
    m_pSyncLeaderEnabled->connectValueChangeRequest(this,
            &ControllerSyncable::slotSyncLeaderEnabledChangeRequest,
            Qt::DirectConnection);
}

void ControllerSyncable::slotSyncLeaderEnabledChangeRequest(double value) {
    SyncMode mode = syncModeFromDouble(value);
    emit syncModeRequested(mode);
}
