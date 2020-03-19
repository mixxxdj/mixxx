#include "preferences/colorpaletteeditor.h"

#include <QColorDialog>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMenu>
#include <QStandardItemModel>
#include <QTableView>

#include "preferences/colorpalettesettings.h"
#include "util/color/predefinedcolorpalettes.h"

namespace {
const QColor kDefaultPaletteColor(0, 0, 0);
}

ColorPaletteEditor::ColorPaletteEditor(QWidget* parent)
        : QWidget(parent),
          m_bPaletteExists(false),
          m_bPaletteIsReadOnly(false),
          m_pPaletteNameComboBox(make_parented<QComboBox>()),
          m_pTableView(make_parented<QTableView>()),
          m_pModel(new ColorPaletteEditorModel(m_pTableView)) {
    m_pPaletteNameComboBox->setEditable(true);

    QDialogButtonBox* pButtonBox = new QDialogButtonBox();
    m_pSaveButton = pButtonBox->addButton(QDialogButtonBox::Save);
    m_pResetButton = pButtonBox->addButton(QDialogButtonBox::Reset);
    m_pDiscardButton = pButtonBox->addButton(QDialogButtonBox::Discard);

    QHBoxLayout* pTopLayout = new QHBoxLayout();
    pTopLayout->addWidget(new QLabel("Name:"));
    pTopLayout->addWidget(m_pPaletteNameComboBox, 1);
    pTopLayout->addWidget(pButtonBox);

    QVBoxLayout* pLayout = new QVBoxLayout();
    pLayout->addLayout(pTopLayout);
    pLayout->addWidget(m_pTableView, 1);
    setLayout(pLayout);
    setContentsMargins(0, 0, 0, 0);

    // Set up model
    m_pModel->setColumnCount(2);
    m_pModel->setHeaderData(0, Qt::Horizontal, tr("Color"), Qt::DisplayRole);
    m_pModel->setHeaderData(1, Qt::Horizontal, tr("Assign to Hotcue"), Qt::DisplayRole);
    connect(m_pModel,
            &ColorPaletteEditorModel::dirtyChanged,
            this,
            &ColorPaletteEditor::slotUpdateButtons);
    connect(m_pModel,
            &ColorPaletteEditorModel::emptyChanged,
            this,
            &ColorPaletteEditor::slotUpdateButtons);

    // Setup up table view
    m_pTableView->setShowGrid(false);
    m_pTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_pTableView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_pTableView->setDragDropMode(QAbstractItemView::InternalMove);
    m_pTableView->setDragDropOverwriteMode(false);
    m_pTableView->setContextMenuPolicy(Qt::CustomContextMenu);
    m_pTableView->setModel(m_pModel);

    connect(m_pTableView,
            &QTableView::doubleClicked,
            this,
            &ColorPaletteEditor::slotTableViewDoubleClicked);
    connect(m_pTableView,
            &QTableView::customContextMenuRequested,
            this,
            &ColorPaletteEditor::slotTableViewContextMenuRequested);
    connect(m_pPaletteNameComboBox,
            &QComboBox::editTextChanged,
            this,
            &ColorPaletteEditor::slotPaletteNameChanged);
    connect(m_pDiscardButton,
            &QPushButton::clicked,
            this,
            &ColorPaletteEditor::slotDiscardButtonClicked);
    connect(m_pSaveButton,
            &QPushButton::clicked,
            this,
            &ColorPaletteEditor::slotSaveButtonClicked);
}

void ColorPaletteEditor::initialize(UserSettingsPointer pConfig) {
    DEBUG_ASSERT(!m_pConfig);
    m_pConfig = pConfig;
    reset();
}

void ColorPaletteEditor::reset() {
    m_pPaletteNameComboBox->clear();
    for (const ColorPalette& palette : mixxx::PredefinedColorPalettes::kPalettes) {
        m_pPaletteNameComboBox->addItem(palette.getName());
    }
    m_pPaletteNameComboBox->insertSeparator(mixxx::PredefinedColorPalettes::kPalettes.size());
    ColorPaletteSettings colorPaletteSettings(m_pConfig);
    for (const QString& paletteName : colorPaletteSettings.getColorPaletteNames()) {
        m_pPaletteNameComboBox->addItem(paletteName);
    }
}

void ColorPaletteEditor::slotUpdateButtons() {
    bool bDirty = m_pModel->isDirty();
    bool bEmpty = m_pModel->isEmpty();
    m_pResetButton->setEnabled(bDirty);
    m_pSaveButton->setEnabled(!m_bPaletteExists || (!m_bPaletteIsReadOnly && bDirty && !bEmpty));
    m_pDiscardButton->setEnabled(m_bPaletteExists && !m_bPaletteIsReadOnly);
}

void ColorPaletteEditor::slotTableViewDoubleClicked(const QModelIndex& index) {
    if (index.isValid() && index.column() == 0) {
        QColor color = QColorDialog::getColor();
        if (color.isValid()) {
            m_pModel->setColor(index.row(), color);
        }
    }
}

void ColorPaletteEditor::slotTableViewContextMenuRequested(const QPoint& pos) {
    QMenu menu(this);

    QAction* pAddAction = menu.addAction("Add");
    QAction* pRemoveAction = menu.addAction("Remove");
    QAction* pAction = menu.exec(m_pTableView->viewport()->mapToGlobal(pos));
    if (pAction == pAddAction) {
        m_pModel->appendRow(kDefaultPaletteColor);
    } else if (pAction == pRemoveAction) {
        QModelIndexList selection = m_pTableView->selectionModel()->selectedRows();

        if (selection.count() > 0) {
            QModelIndex index = selection.at(0);

            //row selected
            int row = index.row();
            m_pModel->removeRow(row);
        }
    }
}

void ColorPaletteEditor::slotPaletteNameChanged(const QString& text) {
    bool bPaletteIsReadOnly = false;
    bool bPaletteExists = false;
    for (const ColorPalette& palette : mixxx::PredefinedColorPalettes::kPalettes) {
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
        if (!m_pModel->isDirty()) {
            bool bPaletteFound = false;
            for (const ColorPalette& palette : mixxx::PredefinedColorPalettes::kPalettes) {
                if (text == palette.getName()) {
                    bPaletteFound = true;
                    m_pModel->setColorPalette(palette);
                    break;
                }
            }
            if (!bPaletteFound) {
                m_pModel->setColorPalette(colorPaletteSettings.getColorPalette(text, mixxx::PredefinedColorPalettes::kDefaultHotcueColorPalette));
            }
        }
    }

    m_bPaletteExists = bPaletteExists;
    m_bPaletteIsReadOnly = bPaletteIsReadOnly;
    slotUpdateButtons();
}

void ColorPaletteEditor::slotDiscardButtonClicked() {
    QString paletteName = m_pPaletteNameComboBox->currentText();
    ColorPaletteSettings colorPaletteSettings(m_pConfig);
    colorPaletteSettings.removePalette(paletteName);
    reset();
    emit paletteRemoved(paletteName);
}

void ColorPaletteEditor::slotSaveButtonClicked() {
    QString paletteName = m_pPaletteNameComboBox->currentText();
    ColorPaletteSettings colorPaletteSettings(m_pConfig);
    colorPaletteSettings.setColorPalette(paletteName, m_pModel->getColorPalette(paletteName));
    m_pModel->setDirty(false);
    reset();
    m_pPaletteNameComboBox->setCurrentText(paletteName);
    emit paletteChanged(paletteName);
}

void ColorPaletteEditor::slotResetButtonClicked() {
    QString paletteName = m_pPaletteNameComboBox->currentText();
    ColorPaletteSettings colorPaletteSettings(m_pConfig);
    bool bPaletteExists = colorPaletteSettings.getColorPaletteNames().contains(paletteName);
    if (!bPaletteExists) {
        for (const ColorPalette& palette : mixxx::PredefinedColorPalettes::kPalettes) {
            if (paletteName == palette.getName()) {
                bPaletteExists = true;
                break;
            }
        }
    }
    m_pModel->setDirty(false);
    reset();
    if (bPaletteExists) {
        m_pPaletteNameComboBox->setCurrentText(paletteName);
    }
}
