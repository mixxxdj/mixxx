#include "preferences/dialog/dlgprefcolors.h"

#include <QColorDialog>
#include <QPainter>
#include <QStandardItemModel>
#include <QtDebug>

#include "control/controlobject.h"
#include "dialog/dlgreplacecuecolor.h"
#include "library/library.h"
#include "library/trackcollection.h"
#include "util/color/predefinedcolorpalettes.h"
#include "util/compatibility.h"
#include "util/math.h"
#include "util/memory.h"

namespace {

constexpr int kHotcueDefaultColorIndex = -1;
constexpr QSize kPalettePreviewSize = QSize(108, 16);
const ConfigKey kAutoHotcueColorsConfigKey("[Controls]", "auto_hotcue_colors");
const ConfigKey kHotcueDefaultColorIndexConfigKey("[Controls]", "HotcueDefaultColorIndex");
} // anonymous namespace

DlgPrefColors::DlgPrefColors(
        QWidget* parent, UserSettingsPointer pConfig, Library* pLibrary)
        : DlgPreferencePage(parent),
          m_pConfig(pConfig),
          m_colorPaletteSettings(ColorPaletteSettings(pConfig)),
          m_pReplaceCueColorDlg(new DlgReplaceCueColor(
                  pConfig,
                  pLibrary->dbConnectionPool(),
                  pLibrary->trackCollections(),
                  this)) {
    setupUi(this);
    comboBoxHotcueColors->setIconSize(kPalettePreviewSize);
    comboBoxTrackColors->setIconSize(kPalettePreviewSize);

    m_pReplaceCueColorDlg->setHidden(true);
    connect(m_pReplaceCueColorDlg,
            &DlgReplaceCueColor::databaseTracksChanged,
            &(pLibrary->trackCollections()->internalCollection()->getTrackDAO()),
            &TrackDAO::slotDatabaseTracksChanged);

    connect(comboBoxHotcueColors,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            &DlgPrefColors::slotHotcuePaletteIndexChanged);

    connect(pushButtonEditHotcuePalette,
            &QPushButton::clicked,
            this,
            &DlgPrefColors::slotEditHotcuePaletteClicked);

    connect(pushButtonEditTrackPalette,
            &QPushButton::clicked,
            this,
            &DlgPrefColors::slotEditTrackPaletteClicked);

    connect(pushButtonReplaceCueColor,
            &QPushButton::clicked,
            this,
            &DlgPrefColors::slotReplaceCueColorClicked);

    loadSettings();
}

DlgPrefColors::~DlgPrefColors() {
}

void DlgPrefColors::slotUpdate() {
    loadSettings();
}

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

    const QSet<QString> colorPaletteNames = m_colorPaletteSettings.getColorPaletteNames();
    for (const auto& paletteName : colorPaletteNames) {
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

    const ColorPalette trackPalette =
            m_colorPaletteSettings.getTrackColorPalette();
    comboBoxTrackColors->setCurrentText(
            trackPalette.getName());

    const ColorPalette hotcuePalette =
            m_colorPaletteSettings.getHotcueColorPalette();
    comboBoxHotcueColors->setCurrentText(
            hotcuePalette.getName());
    slotHotcuePaletteIndexChanged(comboBoxHotcueColors->currentIndex());

    bool autoHotcueColors = m_pConfig->getValue(kAutoHotcueColorsConfigKey, false);
    if (autoHotcueColors) {
        comboBoxHotcueDefaultColor->setCurrentIndex(0);
    } else {
        int hotcueDefaultColorIndex = m_pConfig->getValue(
                kHotcueDefaultColorIndexConfigKey, kHotcueDefaultColorIndex);
        if (hotcueDefaultColorIndex < 0 ||
                hotcueDefaultColorIndex >= hotcuePalette.size()) {
            hotcueDefaultColorIndex =
                    hotcuePalette.size() - 1; // default to last color (orange)
        }
        comboBoxHotcueDefaultColor->setCurrentIndex(
                hotcueDefaultColorIndex + 1);
    }
}

// Set the default values for all the widgets
void DlgPrefColors::slotResetToDefaults() {
    comboBoxHotcueColors->setCurrentText(
            mixxx::PredefinedColorPalettes::kDefaultHotcueColorPalette
                    .getName());
    comboBoxTrackColors->setCurrentText(
            mixxx::PredefinedColorPalettes::kDefaultTrackColorPalette
                    .getName());
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

    for (const auto& palette :
            qAsConst(mixxx::PredefinedColorPalettes::kPalettes)) {
        if (!bHotcueColorPaletteFound &&
                hotcueColorPaletteName == palette.getName()) {
            m_colorPaletteSettings.setHotcueColorPalette(palette);
            bHotcueColorPaletteFound = true;
        }
        if (!bTrackColorPaletteFound &&
                trackColorPaletteName == palette.getName()) {
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
        m_pConfig->setValue(kAutoHotcueColorsConfigKey, false);
        m_pConfig->setValue(kHotcueDefaultColorIndexConfigKey, index - 1);
    } else {
        m_pConfig->setValue(kAutoHotcueColorsConfigKey, true);
        m_pConfig->setValue(kHotcueDefaultColorIndexConfigKey, -1);
    }
}

void DlgPrefColors::slotReplaceCueColorClicked() {
    ColorPaletteSettings colorPaletteSettings(m_pConfig);

    ColorPalette savedPalette = colorPaletteSettings.getHotcueColorPalette();
    ColorPalette newPalette = colorPaletteSettings.getColorPalette(
            comboBoxHotcueColors->currentText(), savedPalette);
    m_pReplaceCueColorDlg->setColorPalette(newPalette);

    int savedDefaultColorIndex = m_pConfig->getValue(
            kHotcueDefaultColorIndexConfigKey, savedPalette.size() - 1);
    if (savedDefaultColorIndex < 0) {
        savedDefaultColorIndex = 0;
    }
    mixxx::RgbColor savedDefaultColor =
            savedPalette.at(savedDefaultColorIndex % savedPalette.size());

    int newDefaultColorIndex = comboBoxHotcueDefaultColor->currentIndex() - 1;
    if (newDefaultColorIndex < 0) {
        newDefaultColorIndex = 0;
    }
    mixxx::RgbColor newDefaultColor = newPalette.at(newDefaultColorIndex % newPalette.size());

    m_pReplaceCueColorDlg->setCurrentColor(savedDefaultColor);
    if (savedDefaultColor != newDefaultColor) {
        m_pReplaceCueColorDlg->setNewColor(newDefaultColor);
    }

    m_pReplaceCueColorDlg->show();
    m_pReplaceCueColorDlg->raise();
    m_pReplaceCueColorDlg->activateWindow();
}

QPixmap DlgPrefColors::drawPalettePreview(const QString& paletteName) {
    ColorPalette palette = m_colorPaletteSettings.getHotcueColorPalette(paletteName);
    if (paletteName == palette.getName()) {
        QPixmap pixmap(kPalettePreviewSize);
        int count = math_max(palette.size(), 1);
        // Rounding up is required so the entire width of the pixmap is filled up to the edge.
        int widthPerColor = static_cast<int>(ceil(pixmap.width() / static_cast<float>(count)));
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

QIcon DlgPrefColors::drawHotcueColorByPaletteIcon(const QString& paletteName) {
    QPixmap pixmap(16, 16);
    QPainter painter(&pixmap);
    pixmap.fill(Qt::black);

    ColorPalette palette = m_colorPaletteSettings.getHotcueColorPalette(paletteName);
    if (paletteName == palette.getName()) {
        for (int i = 0; i < 4; ++i) {
            QColor color = mixxx::RgbColor::toQColor(
                    palette.colorForHotcueIndex(i));
            painter.setPen(color);
            painter.setBrush(color);
            painter.drawRect(0, i * 4, 16, 4);
        }
        return QIcon(pixmap);
    }
    return QIcon();
}

void DlgPrefColors::slotHotcuePaletteIndexChanged(int paletteIndex) {
    QString paletteName = comboBoxHotcueColors->itemText(paletteIndex);
    ColorPalette palette =
            m_colorPaletteSettings.getHotcueColorPalette(paletteName);

    int defaultColor = comboBoxHotcueDefaultColor->currentIndex();
    comboBoxHotcueDefaultColor->clear();

    comboBoxHotcueDefaultColor->addItem(tr("By hotcue number"), -1);
    QIcon icon = drawHotcueColorByPaletteIcon(paletteName);
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

    if (comboBoxHotcueDefaultColor->count() > defaultColor) {
        comboBoxHotcueDefaultColor->setCurrentIndex(defaultColor);
    } else {
        comboBoxHotcueDefaultColor->setCurrentIndex(
                comboBoxHotcueDefaultColor->count() - 1);
    }
}

void DlgPrefColors::slotEditTrackPaletteClicked() {
    QString trackColorPaletteName = comboBoxTrackColors->currentText();
    openColorPaletteEditor(trackColorPaletteName, false);
}

void DlgPrefColors::slotEditHotcuePaletteClicked() {
    QString hotcueColorPaletteName = comboBoxHotcueColors->currentText();
    openColorPaletteEditor(hotcueColorPaletteName, true);
}

void DlgPrefColors::openColorPaletteEditor(
        const QString& paletteName,
        bool editHotcuePalette) {
    std::unique_ptr<ColorPaletteEditor> pColorPaletteEditor =
            std::make_unique<ColorPaletteEditor>(this, editHotcuePalette);

    if (editHotcuePalette) {
        connect(pColorPaletteEditor.get(),
                &ColorPaletteEditor::paletteChanged,
                this,
                &DlgPrefColors::hotcuePaletteUpdated);
    } else {
        connect(pColorPaletteEditor.get(),
                &ColorPaletteEditor::paletteChanged,
                this,
                &DlgPrefColors::trackPaletteUpdated);
    }
    connect(pColorPaletteEditor.get(),
            &ColorPaletteEditor::paletteRemoved,
            this,
            &DlgPrefColors::palettesUpdated);

    pColorPaletteEditor->initialize(m_pConfig, paletteName);
    pColorPaletteEditor->exec();
}

void DlgPrefColors::trackPaletteUpdated(const QString& trackColors) {
    QString hotcueColors = comboBoxHotcueColors->currentText();
    int defaultColor = comboBoxHotcueDefaultColor->currentIndex();

    slotUpdate();
    restoreComboBoxes(hotcueColors, trackColors, defaultColor);
}

void DlgPrefColors::hotcuePaletteUpdated(const QString& hotcueColors) {
    QString trackColors = comboBoxTrackColors->currentText();
    int defaultColor = comboBoxHotcueDefaultColor->currentIndex();

    slotUpdate();
    restoreComboBoxes(hotcueColors, trackColors, defaultColor);
}

void DlgPrefColors::palettesUpdated() {
    QString hotcueColors = comboBoxHotcueColors->currentText();
    QString trackColors = comboBoxTrackColors->currentText();
    int defaultColor = comboBoxHotcueDefaultColor->currentIndex();

    slotUpdate();
    restoreComboBoxes(hotcueColors, trackColors, defaultColor);
}

void DlgPrefColors::restoreComboBoxes(
        const QString& hotcueColors,
        const QString& trackColors,
        int defaultColor) {
    comboBoxHotcueColors->setCurrentText(hotcueColors);
    comboBoxTrackColors->setCurrentText(trackColors);
    if (comboBoxHotcueDefaultColor->count() > defaultColor) {
        comboBoxHotcueDefaultColor->setCurrentIndex(defaultColor);
    } else {
        comboBoxHotcueDefaultColor->setCurrentIndex(
                comboBoxHotcueDefaultColor->count() - 1);
    }
}
