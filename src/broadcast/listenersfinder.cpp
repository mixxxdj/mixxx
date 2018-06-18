
#include "broadcast/listenersfinder.h"

#include <QLinkedList>
#include <QString>

#include "broadcast/filelistener.h"

ListenersFinder& ListenersFinder::instance(UserSettingsPointer pSettings) {
    static ListenersFinder instance(pSettings);
    return instance;
}

ScrobblingServicePtr ListenersFinder::getService(const QString& serviceName) const {
    auto it = m_servicesHash.find(serviceName);
    return it == m_servicesHash.end() ? nullptr : m_servicesHash[serviceName];
}

QLinkedList<ScrobblingServicePtr> ListenersFinder::getAllServices() const {
    QLinkedList<ScrobblingServicePtr> ret;
    for (const auto& servicePtr : m_servicesHash) {
        ret.append(servicePtr);
    }
    return ret;
}

ListenersFinder::ListenersFinder(UserSettingsPointer pSettings) {
    m_servicesHash[fileListenerServiceKey()] =
            ScrobblingServicePtr(FileListener::makeFileListener(FileListener::FileListenerType::SAMBroadcaster, pSettings));
}
