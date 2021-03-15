#pragma once

#include <QObject>

#include "control/controlpushbutton.h"
#include "engine/sync/syncable.h"

class ControllerSyncable : public QObject, public Syncable {
    Q_OBJECT

  public:
    ControllerSyncable(const QString& group);

    const QString& getGroup() const override {
        return m_group;
    };

    EngineChannel* getChannel() const override {
        return nullptr;
    }

    /// Notify a Syncable that their mode has changed. The Syncable must record
    /// this mode and return the latest mode in response to getMode().
    void setSyncMode(SyncMode mode) override {
        m_syncMode = mode;
        m_pSyncLeaderEnabled->setAndConfirm(isMaster(mode));
    }

    /// Must NEVER return a mode that was not set directly via
    /// notifySyncModeChanged.
    SyncMode getSyncMode() const override {
        return m_syncMode;
    }

  signals:
    void syncModeRequested(SyncMode mode);

  protected slots:
    void slotSyncLeaderEnabledChangeRequest(double value);

  protected:
    QString m_group;
    SyncMode m_syncMode;

    QScopedPointer<ControlPushButton> m_pSyncLeaderEnabled;
};
