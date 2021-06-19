#pragma once

#include <QDir>
#include <QFileInfo>
#include <QList>
#include <QPixmap>
#include <QScreen>
#include <QSettings>
#include <QString>

#include "skin/skin.h"

namespace mixxx {
namespace skin {
namespace qml {

class QmlSkin : public mixxx::skin::Skin {
  public:
    QmlSkin() = default;
    QmlSkin(const QFileInfo& path);

    static SkinPointer fromDirectory(const QDir& dir);

    mixxx::skin::SkinType type() const override {
        return mixxx::skin::SkinType::QML;
    };
    bool isValid() const override;
    QFileInfo path() const override;
    QPixmap preview(const QString& schemeName) const override;

    QString name() const override;
    QString description() const override;
    QList<QString> colorschemes() const override;

    bool fitsScreenSize(const QScreen& screen) const override;
    LaunchImage* loadLaunchImage(QWidget* pParent, UserSettingsPointer pConfig) const override;
    QWidget* loadSkin(QWidget* pParent,
            UserSettingsPointer pConfig,
            QSet<ControlObject*>* pSkinCreatedControls,
            mixxx::CoreServices* pCoreServices) const override;

  private:
    QDir dir() const;

    QFileInfo m_path;
    QSettings m_settings;
};

} // namespace qml
} // namespace skin
} // namespace mixxx
