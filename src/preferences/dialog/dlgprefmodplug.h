// dlgprefmodplug.h  -  modplug settings dialog
// created 2013 by Stefan Nuernberger <kabelfrickler@gmail.com>

#ifndef DLGPREFMODPLUG_H
#define DLGPREFMODPLUG_H

#include <QByteArrayData>
#include <QDialog>
#include <QString>

#include "preferences/dlgpreferencepage.h"
#include "preferences/usersettings.h"

class QObject;
class QWidget;

namespace Ui {
class DlgPrefModplug;
}

class DlgPrefModplug : public DlgPreferencePage {
    Q_OBJECT

  public:
    explicit DlgPrefModplug(QWidget* parent, UserSettingsPointer _config);
    virtual ~DlgPrefModplug();

  public slots:
    void slotApply() override;
    void slotUpdate() override;
    void slotResetToDefaults() override;

    void loadSettings();
    void saveSettings();
    void applySettings();

  private:
    Ui::DlgPrefModplug* m_pUi;
    UserSettingsPointer m_pConfig;
};

#endif // DLGPREFMODPLUG_H
