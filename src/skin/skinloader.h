#pragma once

#include <QDir>
#include <QList>
#include <QObject>
#include <QSet>
#include <QWidget>

#include "preferences/usersettings.h"
#include "skin/skin.h"

class ControlProxy;
class ControlPushButton;

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
    QList<SkinPointer> getSkins() const;

  private slots:
    void slotNumMicsChanged(double numMics);

  private:
    QString pickResizableSkin(const QString& oldSkin) const;
    SkinPointer skinFromDirectory(const QDir& dir) const;

    UserSettingsPointer m_pConfig;

    bool m_spinnyCoverControlsCreated;
    void setupSpinnyCoverControls();
    void updateSpinnyCoverControls();
    ControlPushButton* m_pShowSpinny;
    ControlPushButton* m_pShowCover;
    ControlPushButton* m_pShowSpinnyAndOrCover;
    ControlPushButton* m_pSelectBigSpinnyCover;
    ControlPushButton* m_pShowSmallSpinnyCover;
    ControlPushButton* m_pShowBigSpinnyCover;

    bool m_micDuckingControlsCreated;
    void setupMicDuckingControls();
    void updateDuckingControl();
    ControlPushButton* m_pShowDuckingControls;
    ControlProxy* m_pNumMics;
    int m_numMicsEnabled;
    QList<ControlProxy*> m_pMicConfiguredControls;
};

} // namespace skin
} // namespace mixxx
