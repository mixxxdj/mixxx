#pragma once
#include <QHash>

#include "broadcast/scrobblingservice.h"

class ListenersFinder {
  public:
    static ListenersFinder& instance(UserSettingsPointer pSettings);
    static QString fileListenerServiceKey() {
        return "FileListener";
    }
    ScrobblingServicePtr getService(const QString& serviceName) const;
    QLinkedList<ScrobblingServicePtr> getAllServices() const;

  private:
    explicit ListenersFinder(UserSettingsPointer pSettings);
    QHash<QString, ScrobblingServicePtr> m_servicesHash;
};
