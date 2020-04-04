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
    DlgPrefColors(QWidget* parent, UserSettingsPointer pConfig);
    virtual ~DlgPrefColors();

  public slots:
    // Apply changes to widget
    void slotApply();
    void slotResetToDefaults();

  signals:
    void apply(const QString&);

  private:
    void loadSettings();
    void loadPaletteIntoEditor(const ColorPalette& palette);

    const UserSettingsPointer m_pConfig;
    ColorPaletteSettings m_colorPaletteSettings;
};
