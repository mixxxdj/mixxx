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
    void slotEditClicked();
    void slotCloseClicked();

  signals:
    void apply(const QString&);

  private slots:
    void slotHotcuePaletteChanged(const QString& palette);
    void loadSettings();
    void palettesUpdated();

  private:
    void loadPaletteIntoEditor(const ColorPalette& palette);
    QPixmap drawPalettePreview(const QString& paletteName);
    QIcon drawPaletteIcon(const QString& paletteName);

    const UserSettingsPointer m_pConfig;
    ColorPaletteSettings m_colorPaletteSettings;
};
