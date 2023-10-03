#include "qml/qmlplayermanagerproxy.h"

#include <QQmlEngine>

#include "mixer/playermanager.h"
#include "moc_qmlplayermanagerproxy.cpp"
#include "qml/qmlplayerproxy.h"

namespace mixxx {
namespace qml {

QmlPlayerManagerProxy::QmlPlayerManagerProxy(
        std::shared_ptr<PlayerManager> pPlayerManager, QObject* parent)
        : QObject(parent), m_pPlayerManager(pPlayerManager) {
}

QObject* QmlPlayerManagerProxy::getPlayer(const QString& group) {
    BaseTrackPlayer* pPlayer = m_pPlayerManager->getPlayer(group);
    if (!pPlayer) {
        qWarning() << "PlayerManagerProxy failed to find player for group" << group;
        return nullptr;
    }

    // Don't set a parent here, so that the QML engine deletes the object when
    // the corresponding JS object is garbage collected.
    QmlPlayerProxy* pPlayerProxy = new QmlPlayerProxy(pPlayer);
    QQmlEngine::setObjectOwnership(pPlayerProxy, QQmlEngine::JavaScriptOwnership);
    connect(pPlayerProxy,
            &QmlPlayerProxy::loadTrackFromLocationRequested,
            this,
            [this, group](const QString& trackLocation, bool play) {
                loadLocationToPlayer(trackLocation, group, play);
            });
    connect(pPlayerProxy,
            &QmlPlayerProxy::cloneFromGroup,
            this,
            [this, group](const QString& sourceGroup) {
                m_pPlayerManager->slotCloneDeck(sourceGroup, group);
            });
    return pPlayerProxy;
}

void QmlPlayerManagerProxy::loadLocationIntoNextAvailableDeck(
        const QString& trackLocation, bool play) {
    m_pPlayerManager->slotLoadLocationIntoNextAvailableDeck(trackLocation, play);
}

void QmlPlayerManagerProxy::loadLocationUrlIntoNextAvailableDeck(
        const QUrl& trackLocationUrl, bool play) {
    if (trackLocationUrl.isLocalFile()) {
        loadLocationIntoNextAvailableDeck(trackLocationUrl.toLocalFile(), play);
    } else {
        qWarning() << "QmlPlayerManagerProxy: URL" << trackLocationUrl << "is not a local file!";
    }
}

void QmlPlayerManagerProxy::loadLocationToPlayer(
        const QString& location, const QString& group, bool play) {
    m_pPlayerManager->slotLoadLocationToPlayer(location, group, play);
}

// static
QmlPlayerManagerProxy* QmlPlayerManagerProxy::create(QQmlEngine* pQmlEngine, QJSEngine* pJsEngine) {
    Q_UNUSED(pQmlEngine);

    // The implementation of this method is mostly taken from the code example
    // that shows the replacement for `qmlRegisterSingletonInstance()` when
    // using `QML_SINGLETON`.
    // https://doc.qt.io/qt-6/qqmlengine.html#QML_SINGLETON

    // The instance has to exist before it is used. We cannot replace it.
    DEBUG_ASSERT(s_pInstance);

    // The engine has to have the same thread affinity as the singleton.
    DEBUG_ASSERT(pJsEngine->thread() == s_pInstance->thread());

    // There can only be one engine accessing the singleton.
    if (s_pJsEngine) {
        DEBUG_ASSERT(pJsEngine == s_pJsEngine);
    } else {
        s_pJsEngine = pJsEngine;
    }

    // Explicitly specify C++ ownership so that the engine doesn't delete
    // the instance.
    QJSEngine::setObjectOwnership(s_pInstance, QJSEngine::CppOwnership);
    return s_pInstance;
}

} // namespace qml
} // namespace mixxx
