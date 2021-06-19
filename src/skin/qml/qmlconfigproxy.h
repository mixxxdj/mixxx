#pragma once
#include <QColor>
#include <QObject>
#include <QVariantList>

#include "preferences/usersettings.h"

namespace mixxx {
namespace skin {
namespace qml {

class QmlConfigProxy : public QObject {
    Q_OBJECT
  public:
    explicit QmlConfigProxy(
            UserSettingsPointer pConfig,
            QObject* parent = nullptr);

    Q_INVOKABLE QVariantList getHotcueColorPalette();
    Q_INVOKABLE QVariantList getTrackColorPalette();

  private:
    const UserSettingsPointer m_pConfig;
};

} // namespace qml
} // namespace skin
} // namespace mixxx
