#pragma once

#include <QWidget>

#include "control/controlproxy.h"
#include "preferences/colorpaletteeditor.h"
#include "preferences/colorpalettesettings.h"
#include "preferences/dialog/ui_dlgprefcolorsdlg.h"
#include "preferences/dlgpreferencepage.h"
#include "preferences/usersettings.h"
#include "util/parented_ptr.h"

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

  private slots:
    void slotHotcuePaletteChanged(const QString& palette);
    void loadSettings();
    void trackPaletteUpdated(const QString& palette);
    void hotcuePaletteUpdated(const QString& palette);
    void palettesUpdated();
    void slotEditTrackPaletteClicked();
    void slotEditHotcuePaletteClicked();

  private:
    void openColorPaletteEditor(
            const QString& paletteName,
            bool editHotcuePalette);
    QPixmap drawPalettePreview(const QString& paletteName);
    QIcon drawHotcueColorByPaletteIcon(const QString& paletteName);
    void restoreComboBoxes(
            const QString& hotcueColors,
            const QString& trackColors,
            int defaultColor);

    const UserSettingsPointer m_pConfig;
    ColorPaletteSettings m_colorPaletteSettings;
};
