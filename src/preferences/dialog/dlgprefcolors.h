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
    DlgPrefColors(
            QWidget* parent,
            UserSettingsPointer pConfig,
            std::shared_ptr<Library> pLibrary);
    virtual ~DlgPrefColors();

  public slots:
    /// Called when the preference dialog (not this page) is shown to the user.
    void slotUpdate() override;
    /// Called when the user clicks the global "Apply" button.
    void slotApply() override;
    /// Called when the user clicks the global "Reset to Defaults" button.
    void slotResetToDefaults() override;

  signals:
    void apply(const QString&);

  private slots:
    void slotHotcuePaletteIndexChanged(int paletteIndex);
    void trackPaletteUpdated(const QString& palette);
    void hotcuePaletteUpdated(const QString& palette);
    void palettesUpdated();
    void slotReplaceCueColorClicked();
    void slotEditTrackPaletteClicked();
    void slotEditHotcuePaletteClicked();

  private:
    /// Loads the config keys and sets the widgets in the dialog to match
    void loadSettings();
    void openColorPaletteEditor(
            const QString& paletteName,
            bool editHotcuePalette);
    QPixmap drawPalettePreview(const QString& paletteName);
    QIcon drawHotcueColorByPaletteIcon(const QString& paletteName);
    void restoreComboBoxes(
            const QString& hotcueColors,
            const QString& trackColors,
            int defaultHotcueColor,
            int defaultLoopColor);

    const UserSettingsPointer m_pConfig;
    ColorPaletteSettings m_colorPaletteSettings;
    // Pointer to color replace dialog
    DlgReplaceCueColor* m_pReplaceCueColorDlg;
};
