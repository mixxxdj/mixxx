#pragma once

#include <QDialog>
#include <QFile>
#include <QSortFilterProxyModel>

#include "control/controlsortfiltermodel.h"
#include "dialog/ui_dlgdevelopertoolsdlg.h"
#include "preferences/usersettings.h"
#include "util/statmodel.h"

class DlgDeveloperTools : public QDialog, public Ui::DlgDeveloperTools {
    Q_OBJECT
  public:
    DlgDeveloperTools(QWidget* pParent, UserSettingsPointer pConfig);

    bool eventFilter(QObject* pObj, QEvent* pEvent) override;

  protected:
    void timerEvent(QTimerEvent* pTimerEvent) override;

  private slots:
    void slotControlSearch(const QString& search);
    void slotLogSearch();
    void slotControlDump();

  private:
    UserSettingsPointer m_pConfig;
    ControlSortFilterModel m_controlProxyModel;

    StatModel m_statModel;
    QSortFilterProxyModel m_statProxyModel;

    QFile m_logFile;
    QTextCursor m_logCursor;
};
