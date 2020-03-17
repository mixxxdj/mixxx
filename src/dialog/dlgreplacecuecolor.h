#pragma once

#include <QDialog>
#include <QFuture>
#include <QFutureWatcher>
#include <QMap>
#include <QMenu>
#include <QProgressDialog>
#include <QPushButton>
#include <QSet>

#include "dialog/ui_dlgreplacecuecolordlg.h"
#include "preferences/usersettings.h"
#include "track/trackid.h"
#include "util/color/rgbcolor.h"
#include "util/db/dbconnectionpooled.h"
#include "util/db/dbconnectionpooler.h"
#include "widget/wcolorpickeraction.h"

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
            QWidget* pParent);
    ~DlgReplaceCueColor();

  private slots:
    void slotApply();
    void slotDatabaseIdsSelected();
    void slotDatabaseUpdated();

  private:
    void setApplyButtonEnabled(bool enabled);
    QMap<int, int> selectCues(
            mixxx::RgbColor::optional_t currentColor,
            int hotcueIndex,
            Conditions conditions);
    void updateCues(QSet<int> cueIds, QSet<TrackId> trackIds, mixxx::RgbColor newColor);

    const UserSettingsPointer m_pConfig;
    mixxx::DbConnectionPoolPtr m_pDbConnectionPool;
    QMenu* m_pNewColorMenu;
    QMenu* m_pCurrentColorMenu;
    WColorPickerAction* m_pNewColorPickerAction;
    WColorPickerAction* m_pCurrentColorPickerAction;
    QFutureWatcher<QMap<int, int>> m_dbSelectFutureWatcher;
    QFuture<QMap<int, int>> m_dbSelectFuture;
    QFutureWatcher<void> m_dbUpdateFutureWatcher;
    QFuture<void> m_dbUpdateFuture;
};
Q_DECLARE_OPERATORS_FOR_FLAGS(DlgReplaceCueColor::Conditions);
