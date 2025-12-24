#pragma once

#include <QDir>
#include <QList>
#include <QObject>
#include <QSet>

#include "preferences/usersettings.h"
#include "skin/skin.h"
#include "util/parented_ptr.h"

class ControlProxy;
class ControlPushButton;
class QWidget;

namespace mixxx {
namespace skin {

class SkinLoader : public QObject {
    Q_OBJECT
  public:
    SkinLoader(UserSettingsPointer pConfig);
    virtual ~SkinLoader();

    QWidget* loadConfiguredSkin(QWidget* pParent,
            QSet<ControlObject*>* skinCreatedControls,
            mixxx::CoreServices* pCoreServices);

    LaunchImage* loadLaunchImage(QWidget* pParent) const;

    SkinPointer getSkin(const QString& skinName) const;
    SkinPointer getConfiguredSkin() const;
    QString getDefaultSkinName() const;
    QList<QDir> getSkinSearchPaths() const;
    QDir getUserSkinDir() const;
    QDir getSytemSkinDir() const;
    QList<SkinPointer> getUserSkins() const;
    QList<SkinPointer> getSystemSkins() const;

  private slots:
    void slotNumMicsChanged(double numMics);

  private:
    QList<SkinPointer> getSkinsFromDir(const QDir& dir) const;
    QString pickResizableSkin(const QString& oldSkin) const;
    SkinPointer skinFromDirectory(const QDir& dir) const;

    UserSettingsPointer m_pConfig;

    bool m_spinnyCoverControlsCreated;
    void setupSpinnyCoverControls();
    void updateSpinnyCoverControls();
    parented_ptr<ControlProxy> m_pShowSpinny;
    parented_ptr<ControlProxy> m_pShowCover;
    std::unique_ptr<ControlPushButton> m_pShowSpinnyAndOrCover;
    std::unique_ptr<ControlPushButton> m_pSelectBigSpinnyCover;
    std::unique_ptr<ControlPushButton> m_pShowSmallSpinnyCover;
    std::unique_ptr<ControlPushButton> m_pShowBigSpinnyCover;

    bool m_micDuckingControlsCreated;
    void setupMicDuckingControls();
    void updateDuckingControl();
    std::unique_ptr<ControlPushButton> m_pShowDuckingControls;
    parented_ptr<ControlProxy> m_pNumMics;
    int m_numMicsEnabled;
    QList<ControlProxy*> m_pMicConfiguredControls;
};

} // namespace skin
} // namespace mixxx
