#pragma once

#include <QDialog>
#include <QMap>
#include <QMenu>
#include <QProgressDialog>
#include <QPushButton>
#include <QSet>

#include "dialog/ui_dlgreplacecuecolordlg.h"
#include "library/dao/trackdao.h"
#include "library/trackcollectionmanager.h"
#include "preferences/usersettings.h"
#include "track/trackid.h"
#include "util/color/rgbcolor.h"
#include "util/db/dbconnectionpooled.h"
#include "util/db/dbconnectionpooler.h"
#include "widget/wcolorpickeraction.h"

// Dialog for bulk replacing colors of cues in the Database.
class DlgReplaceCueColor : public QDialog, public Ui::DlgReplaceCueColor {
    Q_OBJECT
  public:
    enum class ConditionFlag {
        NoConditions = 0,
        CurrentColorCheck = 1,
        CurrentColorNotEqual = 1 << 1,
        HotcueIndexCheck = 1 << 2,
        HotcueIndexNotEqual = 1 << 3,
    };
    Q_DECLARE_FLAGS(Conditions, ConditionFlag);

    DlgReplaceCueColor(
            UserSettingsPointer pConfig,
            mixxx::DbConnectionPoolPtr dbConnectionPool,
            TrackCollectionManager* pTrackCollectionManager,
            QWidget* pParent);
    ~DlgReplaceCueColor();

  signals:
    void databaseTracksChanged(QSet<TrackId> Trackids);

  private slots:
    void slotApply();
    void reject() override;
    void slotUpdateApplyButton();

  private:
    void setApplyButtonEnabled(bool enabled);

    const UserSettingsPointer m_pConfig;
    mixxx::DbConnectionPoolPtr m_pDbConnectionPool;
    TrackCollectionManager* m_pTrackCollectionManager;
    bool m_bDatabaseChangeInProgress;
    bool m_bDatabaseChanged;
    QMenu* m_pNewColorMenu;
    QMenu* m_pCurrentColorMenu;
    WColorPickerAction* m_pNewColorPickerAction;
    WColorPickerAction* m_pCurrentColorPickerAction;
};
Q_DECLARE_OPERATORS_FOR_FLAGS(DlgReplaceCueColor::Conditions);
