#pragma once

#include <QDialog>
#include <QFuture>
#include <QFutureWatcher>
#include <QProgressDialog>
#include <QPushButton>

#include "dialog/ui_dlgreplacecuecolordlg.h"
#include "preferences/usersettings.h"
#include "util/db/dbconnectionpooled.h"
#include "util/db/dbconnectionpooler.h"

class DlgReplaceCueColor : public QDialog, public Ui::DlgReplaceCueColor {
    Q_OBJECT
  public:
    DlgReplaceCueColor(
            mixxx::DbConnectionPoolPtr dbConnectionPool,
            QWidget* pParent);
    ~DlgReplaceCueColor();

  private slots:
    void slotSelectColor(QPushButton* button);
    void slotApply();
    void slotTransactionFinished();

  private:
    mixxx::DbConnectionPoolPtr m_pDbConnectionPool;
    QFutureWatcher<int> m_dbFutureWatcher;
    QFuture<int> m_dbFuture;
};
