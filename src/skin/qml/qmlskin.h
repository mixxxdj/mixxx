#pragma once

#include <QFileInfo>
#include <QList>
#include <QPixmap>
#include <QString>

#include "skin/skin.h"

class QDir;
class QScreen;

namespace mixxx {
namespace skin {
namespace qml {

class QmlSkin : public mixxx::skin::Skin {
  public:
    explicit QmlSkin(const QFileInfo& path);

    static SkinPointer fromDirectory(const QDir& dir);

    mixxx::skin::SkinType type() const override {
        return mixxx::skin::SkinType::QML;
    }
    bool isValid() const override;
    QFileInfo path() const override;
    QPixmap preview(const QString& schemeName) const override;

    QString name() const override;
    QString displayName() const override;
    QString description() const override;
    QList<QString> colorschemes() const override;

    bool fitsScreenSize(const QScreen& screen) const override;
    LaunchImage* loadLaunchImage(QWidget* pParent, UserSettingsPointer pConfig) const override;
    QWidget* loadSkin(QWidget* pParent,
            UserSettingsPointer pConfig,
            QSet<ControlObject*>* pSkinCreatedControls,
            mixxx::CoreServices* pCoreServices) const override;

    QString mainQmlFilePath() const override;

  private:
    QFileInfo skinIniFile() const;

    QFileInfo m_path;
};

} // namespace qml
} // namespace skin
} // namespace mixxx
