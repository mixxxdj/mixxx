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
  // Apply changes to widget
  void slotApply();
  void slotUpdate();

signals:
  void apply(const QString &);

private:
  void populate();
  void loadSettings();

  ConfigObject<ConfigValue>* m_pConfig;
};

#endif // DLGPREFSELECTOR_H
