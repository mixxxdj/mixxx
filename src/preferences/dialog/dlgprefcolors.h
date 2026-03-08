#pragma once

#include <memory>

#include "preferences/colorpalettesettings.h"
#include "preferences/dialog/dlgpreferencepage.h"
#include "preferences/dialog/ui_dlgprefcolorsdlg.h"
#include "preferences/usersettings.h"

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
    /// Loads the config keys and sets the widgets in the dialog to match
    void slotUpdate() override;
    /// Called when the user clicks the global "Apply" button.
    void slotApply() override;
    /// Called when the user clicks the global "Reset to Defaults" button.
    void slotResetToDefaults() override;

  signals:
    void apply(const QString&);

  private slots:
    void slotHotcuePaletteIndexChanged(int paletteIndex);
    void slotKeyPaletteIndexChanged(int paletteIndex);
    void trackPaletteUpdated(const QString& palette);
    void hotcuePaletteUpdated(const QString& palette);
    void palettesUpdated();
    void slotReplaceCueColorClicked();
    void slotEditTrackPaletteClicked();
    void slotEditHotcuePaletteClicked();
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
    void slotKeyColorsEnabled(Qt::CheckState state);
#else
    void slotKeyColorsEnabled(int i);
#endif

  private:
    void openColorPaletteEditor(
            const QString& paletteName,
            bool editHotcuePalette);
    QPixmap drawPalettePreview(const QString& paletteName);
    QIcon drawHotcueColorByPaletteIcon(const QString& paletteName);
    void restoreComboBoxes(
            const QString& hotcueColors,
            const QString& trackColors,
            int defaultHotcueColor,
            int defaultLoopColor,
            int defaultJumpColor);
    void updateKeyColorsCombobox();

    const UserSettingsPointer m_pConfig;
    ColorPaletteSettings m_colorPaletteSettings;
    bool m_bKeyColorsEnabled;
    // Pointer to color replace dialog
    DlgReplaceCueColor* m_pReplaceCueColorDlg;
};
