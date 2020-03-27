#include "preferences/dialog/dlgprefcolors.h"

#include <QColorDialog>
#include <QPainter>
#include <QStandardItemModel>
#include <QtDebug>

#include "control/controlobject.h"
#include "util/color/predefinedcolorpalettes.h"
#include "util/compatibility.h"
#include "util/math.h"

namespace {

constexpr int kHotcueDefaultColorIndex = -1;
constexpr QSize kPalettePreviewSize = QSize(200, 16);

} // anonymous namespace

DlgPrefColors::DlgPrefColors(
        QWidget* parent, UserSettingsPointer pConfig)
        : DlgPreferencePage(parent),
          m_pConfig(pConfig),
          m_colorPaletteSettings(ColorPaletteSettings(pConfig)) {
    setupUi(this);
    colorPaletteEditor->initialize(pConfig);
    comboBoxHotcueColors->setIconSize(kPalettePreviewSize);
    comboBoxTrackColors->setIconSize(kPalettePreviewSize);

    groupBoxPaletteEditor->hide();

    loadSettings();

    connect(colorPaletteEditor,
            &ColorPaletteEditor::paletteChanged,
            this,
            &DlgPrefColors::loadSettings);
    connect(colorPaletteEditor,
            &ColorPaletteEditor::paletteRemoved,
            this,
            &DlgPrefColors::loadSettings);
    connect(colorPaletteEditor,
            &ColorPaletteEditor::closeButtonClicked,
            this,
            &DlgPrefColors::slotCloseClicked);

    connect(comboBoxHotcueColors,
            QOverload<const QString&>::of(&QComboBox::currentIndexChanged),
            this,
            &DlgPrefColors::slotHotcuePaletteChanged);

    connect(pushButtonEdit,
            &QPushButton::clicked,
            this,
            &DlgPrefColors::slotEditClicked);
}

DlgPrefColors::~DlgPrefColors() {
}

// Loads the config keys and sets the widgets in the dialog to match
void DlgPrefColors::loadSettings() {
    comboBoxHotcueColors->clear();
    comboBoxTrackColors->clear();
    for (const auto& palette : qAsConst(mixxx::PredefinedColorPalettes::kPalettes)) {
        QString paletteName = palette.getName();
        QIcon paletteIcon = drawPalettePreview(paletteName);
        comboBoxHotcueColors->addItem(paletteName);
        comboBoxHotcueColors->setItemIcon(
                comboBoxHotcueColors->count() - 1,
                paletteIcon);

        comboBoxTrackColors->addItem(paletteName);
        comboBoxTrackColors->setItemIcon(
                comboBoxTrackColors->count() - 1,
                paletteIcon);
    }

    for (const auto& paletteName : m_colorPaletteSettings.getColorPaletteNames()) {
        QIcon paletteIcon = drawPalettePreview(paletteName);
        comboBoxHotcueColors->addItem(paletteName);
        comboBoxHotcueColors->setItemIcon(
                comboBoxHotcueColors->count() - 1,
                paletteIcon);
        comboBoxTrackColors->addItem(paletteName);
        comboBoxTrackColors->setItemIcon(
                comboBoxHotcueColors->count() - 1,
                paletteIcon);
    }

    const ColorPalette hotcuePalette =
            m_colorPaletteSettings.getHotcueColorPalette();
    comboBoxHotcueColors->setCurrentText(
            hotcuePalette.getName());
    slotHotcuePaletteChanged(hotcuePalette.getName());

    const ColorPalette trackPalette =
            m_colorPaletteSettings.getTrackColorPalette();
    comboBoxTrackColors->setCurrentText(
            trackPalette.getName());
}

// Set the default values for all the widgets
void DlgPrefColors::slotResetToDefaults() {
    comboBoxHotcueColors->setCurrentText(
            mixxx::PredefinedColorPalettes::kDefaultHotcueColorPalette.getName());
    comboBoxTrackColors->setCurrentText(
            mixxx::PredefinedColorPalettes::kDefaultTrackColorPalette.getName());
    comboBoxHotcueDefaultColor->setCurrentIndex(
            mixxx::PredefinedColorPalettes::kDefaultTrackColorPalette.size());
    slotApply();
}

// Apply and save any changes made in the dialog
void DlgPrefColors::slotApply() {
    QString hotcueColorPaletteName = comboBoxHotcueColors->currentText();
    QString trackColorPaletteName = comboBoxTrackColors->currentText();
    bool bHotcueColorPaletteFound = false;
    bool bTrackColorPaletteFound = false;

    for (const auto& palette : qAsConst(mixxx::PredefinedColorPalettes::kPalettes)) {
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

QPixmap DlgPrefColors::drawPalettePreview(const QString& paletteName) {
    ColorPalette palette = m_colorPaletteSettings.getHotcueColorPalette(paletteName);
    if (paletteName == palette.getName()) {
        QPixmap pixmap(kPalettePreviewSize);
        int count = math_max(palette.size(), 1);
        int widthPerColor = pixmap.width() / count;
        pixmap.fill(Qt::black);
        QPainter painter(&pixmap);
        for (int i = 0; i < palette.size(); ++i) {
            painter.setPen(mixxx::RgbColor::toQColor(palette.at(i)));
            painter.setBrush(mixxx::RgbColor::toQColor(palette.at(i)));
            painter.drawRect(i * widthPerColor, 0, widthPerColor, pixmap.height());
        }
        return pixmap;
    }
    return QPixmap();
}

QIcon DlgPrefColors::drawPaletteIcon(const QString& paletteName) {
    QPixmap pixmap(16, 16);
    QPainter painter(&pixmap);
    pixmap.fill(Qt::black);

    ColorPalette palette = m_colorPaletteSettings.getHotcueColorPalette(paletteName);
    if (paletteName == palette.getName()) {
        for (int i = 0; i < palette.size() && i < 4; ++i) {
            painter.setPen(mixxx::RgbColor::toQColor(palette.at(i)));
            painter.setBrush(mixxx::RgbColor::toQColor(palette.at(i)));
            painter.drawRect(0, i * 4, 16, 4);
        }
        return QIcon(pixmap);
    }
    return QIcon();
}

void DlgPrefColors::slotHotcuePaletteChanged(const QString& paletteName) {
    ColorPalette palette =
            m_colorPaletteSettings.getHotcueColorPalette(paletteName);

    comboBoxHotcueDefaultColor->clear();

    comboBoxHotcueDefaultColor->addItem(tr("By hotcue number"), -1);
    QIcon icon = drawPaletteIcon(paletteName);
    comboBoxHotcueDefaultColor->setItemIcon(0, icon);

    QPixmap pixmap(16, 16);
    for (int i = 0; i < palette.size(); ++i) {
        QColor color = mixxx::RgbColor::toQColor(palette.at(i));
        comboBoxHotcueDefaultColor->addItem(
                tr("Color") +
                        QStringLiteral(" ") +
                        QString::number(i + 1) +
                        QStringLiteral(": ") +
                        color.name(),
                i);
        pixmap.fill(color);
        comboBoxHotcueDefaultColor->setItemIcon(i + 1, QIcon(pixmap));
    }

    bool autoHotcueColors =
            m_pConfig->getValue(ConfigKey("[Controls]", "auto_hotcue_colors"), false);
    if (autoHotcueColors) {
        comboBoxHotcueDefaultColor->setCurrentIndex(0);
    } else {
        int hotcueDefaultColorIndex = m_pConfig->getValue(ConfigKey("[Controls]", "HotcueDefaultColorIndex"), kHotcueDefaultColorIndex);
        if (hotcueDefaultColorIndex < 0 || hotcueDefaultColorIndex >= palette.size()) {
            hotcueDefaultColorIndex = palette.size() - 1; // default to last color (orange)
        }
        comboBoxHotcueDefaultColor->setCurrentIndex(hotcueDefaultColorIndex + 1);
    }
}

void DlgPrefColors::slotEditClicked() {
    pushButtonEdit->hide();
    labelCustomPalette->hide();
    widgetSpacer->hide();
    groupBoxPaletteEditor->show();
}

void DlgPrefColors::slotCloseClicked() {
    groupBoxPaletteEditor->hide();
    widgetSpacer->show();
    pushButtonEdit->show();
    labelCustomPalette->show();
}
