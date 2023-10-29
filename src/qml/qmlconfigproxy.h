#pragma once
#include <QColor>
#include <QObject>
#include <QVariantList>
#include <QtQml>

#include "preferences/usersettings.h"

namespace mixxx {
namespace qml {

class QmlConfigProxy : public QObject {
    Q_OBJECT
    QML_NAMED_ELEMENT(Config)
    QML_SINGLETON
  public:
    explicit QmlConfigProxy(
            UserSettingsPointer pConfig,
            QObject* parent = nullptr);

    Q_INVOKABLE QVariantList getHotcueColorPalette();
    Q_INVOKABLE QVariantList getTrackColorPalette();

    static QmlConfigProxy* create(QQmlEngine* pQmlEngine, QJSEngine* pJsEngine);
    static inline void registerUserSettings(UserSettingsPointer pConfig) {
        s_pUserSettings = std::move(pConfig);
    }

  private:
    static inline UserSettingsPointer s_pUserSettings = nullptr;

    const UserSettingsPointer m_pConfig;
};

} // namespace qml
} // namespace mixxx
