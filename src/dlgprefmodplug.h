// dlgprefmodplug.h  -  modplug settings dialog
// created 2013 by Stefan Nuernberger <kabelfrickler@gmail.com>

#ifndef DLGPREFMODPLUG_H
#define DLGPREFMODPLUG_H

#include <QDialog>
#include "configobject.h"
#include "preferences/dlgpreferencepage.h"

namespace Ui {
class DlgPrefModplug;
}

class DlgPrefModplug : public DlgPreferencePage {
    Q_OBJECT

  public:
    explicit DlgPrefModplug(QWidget* parent,
                            ConfigObject<ConfigValue>* _config);
    virtual ~DlgPrefModplug();

  public slots:
    /** Apply changes to widget */
    void slotApply();
    void slotUpdate();
    void slotResetToDefaults();

    void loadSettings();
    void saveSettings();
    void applySettings();

  private:
    Ui::DlgPrefModplug* m_pUi;
    ConfigObject<ConfigValue>* m_pConfig;
};

#endif  // DLGPREFMODPLUG_H
