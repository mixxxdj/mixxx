#pragma once

#include <QDialog>
#include <QFuture>
#include <QFutureWatcher>
#include <QMenu>
#include <QProgressDialog>
#include <QPushButton>

#include "dialog/ui_dlgreplacecuecolordlg.h"
#include "preferences/usersettings.h"
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
    void slotTransactionFinished();

  private:
    int updateCueColors(
            mixxx::RgbColor::optional_t newColor,
            mixxx::RgbColor::optional_t currentColor,
            int hotcueIndex,
            Conditions conditions);

    const UserSettingsPointer m_pConfig;
    mixxx::DbConnectionPoolPtr m_pDbConnectionPool;
    QMenu* m_pNewColorMenu;
    QMenu* m_pCurrentColorMenu;
    WColorPickerAction* m_pNewColorPickerAction;
    WColorPickerAction* m_pCurrentColorPickerAction;
    QFutureWatcher<int> m_dbFutureWatcher;
    QFuture<int> m_dbFuture;
};
Q_DECLARE_OPERATORS_FOR_FLAGS(DlgReplaceCueColor::Conditions);
