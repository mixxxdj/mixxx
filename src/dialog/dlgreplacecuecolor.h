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
    DlgReplaceCueColor(
            UserSettingsPointer pConfig,
            mixxx::DbConnectionPoolPtr dbConnectionPool,
            QWidget* pParent);
    ~DlgReplaceCueColor();

  private slots:
    void slotApply();
    void slotTransactionFinished();

  private:
    const UserSettingsPointer m_pConfig;
    mixxx::DbConnectionPoolPtr m_pDbConnectionPool;
    QMenu* m_pNewColorMenu;
    QMenu* m_pCurrentColorMenu;
    WColorPickerAction* m_pNewColorPickerAction;
    WColorPickerAction* m_pCurrentColorPickerAction;
    QFutureWatcher<int> m_dbFutureWatcher;
    QFuture<int> m_dbFuture;
};
