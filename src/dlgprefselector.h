#ifndef DLGPREFSELECTOR_H
#define DLGPREFSELECTOR_H

#include <QWidget>

#include "ui_dlgprefselectordlg.h"
#include "configobject.h"


class DlgPrefSelector : public QWidget, Ui::DlgPrefSelectorDlg {
    Q_OBJECT
    
  public:
    DlgPrefSelector(QWidget *parent, ConfigObject<ConfigValue> *pConfig);
    ~DlgPrefSelector();
    
  public slots:
    // Apply changes to preference widget
    void slotApply();
    void slotUpdate();

  signals:
    void apply(const QString &);

  private:
    void loadSettings();

    ConfigObject<ConfigValue>* m_pConfig;
    bool m_bFilterGenre;
    bool m_bFilterBpm;
    int m_iFilterBpmRange;
    bool m_bFilterKey;
    bool m_bFilterKey4th;
    bool m_bFilterKey5th;
    bool m_bFilterKeyRelative;
};

#endif // DLGPREFSELECTOR_H
