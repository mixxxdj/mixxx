// dlgprefmodplug.h  -  modplug settings dialog
// created 2013 by Stefan Nuernberger <kabelfrickler@gmail.com>

#ifndef DLGPREFMODPLUG_H
#define DLGPREFMODPLUG_H

#include <QDialog>
#include "configobject.h"

namespace Ui {
class DlgPrefModplug;
}

class DlgPrefModplug : public QDialog
{
    Q_OBJECT

  public:
    explicit DlgPrefModplug(QWidget *parent, ConfigObject<ConfigValue> *_config);
    ~DlgPrefModplug();

  public slots:
    /** Apply changes to widget */
   void slotApply();
   void slotUpdate();

   void loadSettings();
   void saveSettings();
   void applySettings();

  private:
    Ui::DlgPrefModplug *m_pUi;
    ConfigObject<ConfigValue> *m_pConfig;
};

#endif // DLGPREFMODPLUG_H
