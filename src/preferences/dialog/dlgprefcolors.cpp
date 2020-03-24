#include "preferences/dialog/dlgprefcolors.h"

#include <QColorDialog>
#include <QPainter>
#include <QStandardItemModel>
#include <QtDebug>

#include "control/controlobject.h"
#include "util/color/predefinedcolorpalettes.h"

namespace {

constexpr int kHotcueDefaultColorIndex = -1;

} // anonymous namespace

DlgPrefColors::DlgPrefColors(
        QWidget* parent, UserSettingsPointer pConfig)
        : DlgPreferencePage(parent),
          m_pConfig(pConfig),
          m_colorPaletteSettings(ColorPaletteSettings(pConfig)) {
    setupUi(this);
    colorPaletteEditor->initialize(pConfig);

    loadSettings();

    connect(colorPaletteEditor,
            &ColorPaletteEditor::paletteChanged,
            [this] {
                loadSettings();
            });
    connect(colorPaletteEditor,
            &ColorPaletteEditor::paletteRemoved,
            [this] {
                loadSettings();
            });
}

DlgPrefColors::~DlgPrefColors() {
}

// Loads the config keys and sets the widgets in the dialog to match
void DlgPrefColors::loadSettings() {
    comboBoxHotcueColors->clear();
    comboBoxTrackColors->clear();
    foreach (const ColorPalette& palette, mixxx::PredefinedColorPalettes::kPalettes) {
        QString paletteName = palette.getName();
        comboBoxHotcueColors->addItem(paletteName);
        comboBoxTrackColors->addItem(paletteName);
    }

    foreach (const QString& paletteName, m_colorPaletteSettings.getColorPaletteNames()) {
        comboBoxHotcueColors->addItem(paletteName);
        comboBoxTrackColors->addItem(paletteName);
    }

    const ColorPalette hotcuePalette = m_colorPaletteSettings.getHotcueColorPalette();

    comboBoxHotcueColors->setCurrentText(
            hotcuePalette.getName());
    comboBoxTrackColors->setCurrentText(
            m_colorPaletteSettings.getTrackColorPalette().getName());

    QPixmap pixmap(80, 80);
    QPainter painter(&pixmap);
    pixmap.fill(Qt::black);

    comboBoxHotcueDefaultColor->addItem(tr("By hotcue number"), -1);

    for (int i = 0; i < hotcuePalette.size() && i < 4; ++i) {
        painter.setBrush(mixxx::RgbColor::toQColor(hotcuePalette.at(i)));
        painter.drawRect(0, i * 20, 80, 20);
    }
    comboBoxHotcueDefaultColor->setItemIcon(0, QIcon(pixmap));

    for (int i = 0; i < hotcuePalette.size(); ++i) {
        comboBoxHotcueDefaultColor->addItem(tr("Palette") +
                        QStringLiteral(" ") + QString::number(i + 1),
                i);
        pixmap.fill(mixxx::RgbColor::toQColor(hotcuePalette.at(i)));
        comboBoxHotcueDefaultColor->setItemIcon(i + 1, QIcon(pixmap));
    }

    bool autoHotcueColors =
            m_pConfig->getValue(ConfigKey("[Controls]", "auto_hotcue_colors"), false);
    if (autoHotcueColors) {
        comboBoxHotcueDefaultColor->setCurrentIndex(0);
    } else {
        int hotcueDefaultColorIndex = m_pConfig->getValue(ConfigKey("[Controls]", "HotcueDefaultColorIndex"), kHotcueDefaultColorIndex);
        if (hotcueDefaultColorIndex < 0 || hotcueDefaultColorIndex >= hotcuePalette.size()) {
            hotcueDefaultColorIndex = hotcuePalette.size() - 1; // default to last color (orange)
        }
        comboBoxHotcueDefaultColor->setCurrentIndex(hotcueDefaultColorIndex + 1);
    }
}

// Set the default values for all the widgets
void DlgPrefColors::slotResetToDefaults() {
    comboBoxHotcueColors->setCurrentText(
            mixxx::PredefinedColorPalettes::kDefaultHotcueColorPalette.getName());
    comboBoxTrackColors->setCurrentText(
            mixxx::PredefinedColorPalettes::kDefaultTrackColorPalette.getName());
    comboBoxHotcueDefaultColor->setCurrentIndex(1);
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

    int index = comboBoxHotcueDefaultColor->currentIndex();

    if (index > 0) {
        m_pConfig->setValue(ConfigKey("[Controls]", "auto_hotcue_colors"), false);
        m_pConfig->setValue(ConfigKey("[Controls]", "HotcueDefaultColorIndex"), index - 1);
    } else {
        m_pConfig->setValue(ConfigKey("[Controls]", "auto_hotcue_colors"), true);
        m_pConfig->setValue(ConfigKey("[Controls]", "HotcueDefaultColorIndex"), -1);
    }
}
