#pragma once

#include <QDialog>
#include <QFile>
#include <QSortFilterProxyModel>
#include <QTimerEvent>

#include "control/controlmodel.h"
#include "control/controlobject.h"
#include "dialog/ui_dlgdevelopertoolsdlg.h"
#include "preferences/usersettings.h"
#include "util/statmodel.h"

class DlgDeveloperTools : public QDialog, public Ui::DlgDeveloperTools {
    Q_OBJECT
  public:
    DlgDeveloperTools(QWidget* pParent, UserSettingsPointer pConfig);

  protected:
    void timerEvent(QTimerEvent* pTimerEvent) override;

  private slots:
    void slotControlSearch(const QString& search);
    void slotLogSearch();
    void slotControlDump();

  private:
    UserSettingsPointer m_pConfig;
    ControlModel m_controlModel;
    QSortFilterProxyModel m_controlProxyModel;

    StatModel m_statModel;
    QSortFilterProxyModel m_statProxyModel;

    QFile m_logFile;
    QTextCursor m_logCursor;
};
