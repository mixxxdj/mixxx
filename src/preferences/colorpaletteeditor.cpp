#include "preferences/colorpaletteeditor.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QStandardItemModel>
#include <QTableView>

#include "preferences/colorpalettesettings.h"
#include "util/color/predefinedcolorpalettes.h"

ColorPaletteEditor::ColorPaletteEditor(QWidget* parent)
        : QWidget(parent),
          m_pPaletteNameComboBox(make_parented<QComboBox>()),
          m_pSaveButton(make_parented<QPushButton>("Save")),
          m_pDeleteButton(make_parented<QPushButton>("Delete")),
          m_pResetButton(make_parented<QPushButton>("Reset")),
          m_pTableView(make_parented<ColorPaletteEditorTableView>()) {
    m_pPaletteNameComboBox->setEditable(true);

    QHBoxLayout* pTopLayout = new QHBoxLayout();
    pTopLayout->addWidget(new QLabel("Name:"));
    pTopLayout->addWidget(m_pPaletteNameComboBox, 1);
    pTopLayout->addWidget(m_pSaveButton);
    pTopLayout->addWidget(m_pDeleteButton);
    pTopLayout->addWidget(m_pResetButton);

    QVBoxLayout* pLayout = new QVBoxLayout();
    pLayout->addLayout(pTopLayout);
    pLayout->addWidget(m_pTableView, 1);
    setLayout(pLayout);
    setContentsMargins(0, 0, 0, 0);

    connect(m_pPaletteNameComboBox,
            &QComboBox::editTextChanged,
            [this](const QString& text) {
                bool bPaletteIsReadOnly = false;
                bool bPaletteExists = false;
                foreach (const ColorPalette& palette, mixxx::PredefinedColorPalettes::kPalettes) {
                    if (text == palette.getName()) {
                        bPaletteExists = true;
                        bPaletteIsReadOnly = true;
                        break;
                    }
                }

                ColorPaletteSettings colorPaletteSettings(m_pConfig);
                if (!bPaletteIsReadOnly) {
                    bPaletteExists = colorPaletteSettings.getColorPaletteNames().contains(text);
                }

                if (bPaletteExists) {
                    m_pResetButton->setEnabled(m_pTableView->isDirty());
                    if (!m_pTableView->isDirty()) {
                        bool bPaletteFound = false;
                        foreach (const ColorPalette& palette, mixxx::PredefinedColorPalettes::kPalettes) {
                            if (text == palette.getName()) {
                                bPaletteFound = true;
                                m_pTableView->setColors(palette.getColorList());
                                break;
                            }
                        }
                        if (!bPaletteFound) {
                            m_pTableView->setColors(colorPaletteSettings.getColorPalette(text, mixxx::PredefinedColorPalettes::kDefaultHotcueColorPalette).getColorList());
                        }
                    }
                } else {
                    m_pResetButton->setEnabled(true);
                }
                m_pSaveButton->setEnabled(!text.isEmpty() && !bPaletteIsReadOnly);
                m_pDeleteButton->setEnabled(bPaletteExists && !bPaletteIsReadOnly);
            });

    connect(m_pDeleteButton,
            &QPushButton::clicked,
            [this] {
                QString paletteName = m_pPaletteNameComboBox->currentText();
                ColorPaletteSettings colorPaletteSettings(m_pConfig);
                colorPaletteSettings.removePalette(paletteName);
                reset();
                emit paletteRemoved(paletteName);
            });
    connect(m_pSaveButton,
            &QPushButton::clicked,
            [this] {
                QString paletteName = m_pPaletteNameComboBox->currentText();
                ColorPaletteSettings colorPaletteSettings(m_pConfig);
                colorPaletteSettings.setColorPalette(paletteName, ColorPalette(paletteName, m_pTableView->getColors()));
                m_pTableView->setDirty(false);
                reset();
                m_pPaletteNameComboBox->setCurrentText(paletteName);
                emit paletteChanged(paletteName);
            });
    connect(m_pResetButton,
            &QPushButton::clicked,
            [this] {
                QString paletteName = m_pPaletteNameComboBox->currentText();
                ColorPaletteSettings colorPaletteSettings(m_pConfig);
                bool bPaletteExists = colorPaletteSettings.getColorPaletteNames().contains(paletteName);
                if (!bPaletteExists) {
                    foreach (const ColorPalette& palette, mixxx::PredefinedColorPalettes::kPalettes) {
                        if (paletteName == palette.getName()) {
                            bPaletteExists = true;
                            break;
                        }
                    }
                }
                m_pTableView->setDirty(false);
                reset();
                if (bPaletteExists) {
                    m_pPaletteNameComboBox->setCurrentText(paletteName);
                }
            });
    connect(m_pTableView,
            &ColorPaletteEditorTableView::dirtyChanged,
            [this](bool bDirty) {
                m_pResetButton->setEnabled(bDirty);
            });
}

void ColorPaletteEditor::reset() {
    m_pPaletteNameComboBox->clear();
    foreach (const ColorPalette& palette, mixxx::PredefinedColorPalettes::kPalettes) {
        m_pPaletteNameComboBox->addItem(palette.getName());
    }
    m_pPaletteNameComboBox->insertSeparator(mixxx::PredefinedColorPalettes::kPalettes.size());
    ColorPaletteSettings colorPaletteSettings(m_pConfig);
    foreach (const QString& paletteName, colorPaletteSettings.getColorPaletteNames()) {
        m_pPaletteNameComboBox->addItem(paletteName);
    }
}
