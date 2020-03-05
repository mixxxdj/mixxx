#include "preferences/dialog/dlgprefcolors.h"

#include <QtDebug>

#include "control/controlobject.h"
#include "util/color/predefinedcolorpalettes.h"

DlgPrefColors::DlgPrefColors(
        QWidget* parent, UserSettingsPointer config)
        : DlgPreferencePage(parent),
          m_config(config),
          m_colorPaletteSettings(ColorPaletteSettings(config)) {
    setupUi(this);

    loadSettings();
}

DlgPrefColors::~DlgPrefColors() {
}

// Loads the config keys and sets the widgets in the dialog to match
void DlgPrefColors::loadSettings() {
    foreach (const ColorPalette& palette, mixxx::PredefinedColorPalettes::kPalettes) {
        QString paletteName = palette.getName();
        comboBoxHotcueColors->addItem(paletteName);
        comboBoxTrackColors->addItem(paletteName);
    }

    foreach (const QString& paletteName, m_colorPaletteSettings.getColorPaletteNames()) {
        comboBoxHotcueColors->addItem(paletteName);
        comboBoxTrackColors->addItem(paletteName);
    }

    comboBoxHotcueColors->setCurrentText(
            m_colorPaletteSettings.getHotcueColorPalette().getName());
    comboBoxTrackColors->setCurrentText(
            m_colorPaletteSettings.getTrackColorPalette().getName());
    slotApply();
}

// Set the default values for all the widgets
void DlgPrefColors::slotResetToDefaults() {
    comboBoxHotcueColors->setCurrentText(
            mixxx::PredefinedColorPalettes::kDefaultHotcueColorPalette.getName());
    comboBoxTrackColors->setCurrentText(
            mixxx::PredefinedColorPalettes::kDefaultTrackColorPalette.getName());
    slotApply();
}

// Apply and save any changes made in the dialog
void DlgPrefColors::slotApply() {
    QString hotcueColorPaletteName = comboBoxHotcueColors->currentText();
    QString trackColorPaletteName = comboBoxTrackColors->currentText();
    bool bHotcueColorPaletteFound = false;
    bool bTrackColorPaletteFound = false;

    foreach (const ColorPalette& palette, mixxx::PredefinedColorPalettes::kPalettes) {
        if (!bHotcueColorPaletteFound && hotcueColorPaletteName == palette.getName()) {
            m_colorPaletteSettings.setHotcueColorPalette(palette);
            bHotcueColorPaletteFound = true;
        }
        if (!bTrackColorPaletteFound && trackColorPaletteName == palette.getName()) {
            m_colorPaletteSettings.setTrackColorPalette(palette);
            bTrackColorPaletteFound = true;
        }
    }

    if (!bHotcueColorPaletteFound) {
        m_colorPaletteSettings.setHotcueColorPalette(
                m_colorPaletteSettings.getColorPalette(hotcueColorPaletteName,
                        m_colorPaletteSettings.getHotcueColorPalette()));
    }

    if (!bTrackColorPaletteFound) {
        m_colorPaletteSettings.setTrackColorPalette(
                m_colorPaletteSettings.getColorPalette(trackColorPaletteName,
                        m_colorPaletteSettings.getTrackColorPalette()));
    }
}
