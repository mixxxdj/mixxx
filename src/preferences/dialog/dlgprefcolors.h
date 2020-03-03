#pragma once

#include <QWidget>

#include "control/controlproxy.h"
#include "preferences/colorpalettesettings.h"
#include "preferences/dialog/ui_dlgprefcolorsdlg.h"
#include "preferences/dlgpreferencepage.h"
#include "preferences/usersettings.h"

class DlgPrefColors : public DlgPreferencePage, public Ui::DlgPrefColorsDlg {
    Q_OBJECT
  public:
    DlgPrefColors(QWidget* parent, UserSettingsPointer _config);
    virtual ~DlgPrefColors();

  public slots:
    // Apply changes to widget
    void slotApply();
    void slotResetToDefaults();

  signals:
    void apply(const QString&);

  private:
    void loadSettings();

    // Pointer to config object
    UserSettingsPointer m_config;
    ColorPaletteSettings m_colorPaletteSettings;
};
