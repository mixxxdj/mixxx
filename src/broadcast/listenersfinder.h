#pragma once
#include <QHash>

#include "broadcast/scrobblingservice.h"

namespace {
const QString kfileListenerKey = "FileListener";
const QString klistenbrainzListenerKey = "ListenBrainz";
} // namespace

class ListenersFinder {
  public:
    static ListenersFinder& instance(UserSettingsPointer pSettings);
    ScrobblingServicePtr getService(const QString& serviceName) const;
    QLinkedList<ScrobblingServicePtr> getAllServices() const;

  private:
    explicit ListenersFinder(UserSettingsPointer pSettings);
    QHash<QString, ScrobblingServicePtr> m_servicesHash;
};
