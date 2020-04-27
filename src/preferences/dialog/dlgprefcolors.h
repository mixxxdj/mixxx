#pragma once

#include <QWidget>

#include "control/controlproxy.h"
#include "preferences/colorpaletteeditor.h"
#include "preferences/colorpalettesettings.h"
#include "preferences/dialog/ui_dlgprefcolorsdlg.h"
#include "preferences/dlgpreferencepage.h"
#include "preferences/usersettings.h"
#include "util/parented_ptr.h"

class DlgReplaceCueColor;
class Library;

class DlgPrefColors : public DlgPreferencePage, public Ui::DlgPrefColorsDlg {
    Q_OBJECT
  public:
    DlgPrefColors(QWidget* parent, UserSettingsPointer pConfig, Library* pLibrary);
    virtual ~DlgPrefColors();

  public slots:
    /// Called when the preference dialog (not this page) is shown to the user.
    void slotUpdate();
    /// Called when the user clicks the global "Apply" button.
    void slotApply();
    /// Called when the user clicks the global "Reset to Defaults" button.
    void slotResetToDefaults();

  signals:
    void apply(const QString&);

  private slots:
    void slotHotcuePaletteChanged(const QString& palette);
    void trackPaletteUpdated(const QString& palette);
    void hotcuePaletteUpdated(const QString& palette);
    void palettesUpdated();
    void slotReplaceCueColorClicked();
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
    // Pointer to color replace dialog
    DlgReplaceCueColor* m_pReplaceCueColorDlg;
};
