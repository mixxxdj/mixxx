#include "preferences/colorpaletteeditor.h"

#include <QColorDialog>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMenu>
#include <QMessageBox>
#include <QStandardItemModel>
#include <QTableView>

#include "preferences/colorpalettesettings.h"
#include "util/color/predefinedcolorpalettes.h"

namespace {
const QColor kDefaultPaletteColor(0, 0, 0);
}

ColorPaletteEditor::ColorPaletteEditor(QWidget* parent)
        : QDialog(parent),
          m_bPaletteExists(false),
          m_bPaletteIsReadOnly(false),
          m_pSaveAsEdit(make_parented<QLineEdit>(this)),
          m_pTableView(make_parented<QTableView>(this)),
          m_pModel(make_parented<ColorPaletteEditorModel>(m_pTableView)) {
    QDialogButtonBox* pButtonBox = new QDialogButtonBox();
    m_pRemoveButton = pButtonBox->addButton(
            tr("Remove Palette"),
            QDialogButtonBox::DestructiveRole);
    m_pCloseButton = pButtonBox->addButton(QDialogButtonBox::Discard);
    m_pResetButton = pButtonBox->addButton(QDialogButtonBox::Reset);
    m_pSaveButton = pButtonBox->addButton(QDialogButtonBox::Save);

    QHBoxLayout* pNameLayout = new QHBoxLayout();
    pNameLayout->addWidget(new QLabel(tr("Name")));
    pNameLayout->addWidget(m_pSaveAsEdit, 1);

    QVBoxLayout* pLayout = new QVBoxLayout();
    pLayout->addWidget(m_pTableView, 1);
    pLayout->addLayout(pNameLayout);
    pLayout->addWidget(pButtonBox);
    setLayout(pLayout);
    setContentsMargins(0, 0, 0, 0);

    // Set up model
    m_pModel->setColumnCount(3);
    m_pModel->setHeaderData(0, Qt::Horizontal, tr("Color"), Qt::DisplayRole);
    m_pModel->setHeaderData(1, Qt::Horizontal, tr("Assign to Hotcue Number"), Qt::DisplayRole);
    m_pModel->setHeaderData(2, Qt::Horizontal, QString(), Qt::DisplayRole);
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

    m_pTableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_pTableView->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_pTableView->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);

    connect(m_pTableView,
            &QTableView::doubleClicked,
            this,
            &ColorPaletteEditor::slotTableViewDoubleClicked);
    connect(m_pTableView,
            &QTableView::customContextMenuRequested,
            this,
            &ColorPaletteEditor::slotTableViewContextMenuRequested);
    connect(m_pSaveAsEdit,
            &QLineEdit::textChanged,
            this,
            &ColorPaletteEditor::slotPaletteNameChanged);
    connect(m_pResetButton,
            &QPushButton::clicked,
            this,
            &ColorPaletteEditor::slotResetButtonClicked);
    connect(m_pCloseButton,
            &QPushButton::clicked,
            this,
            &ColorPaletteEditor::slotCloseButtonClicked);
    connect(m_pSaveButton,
            &QPushButton::clicked,
            this,
            &ColorPaletteEditor::slotSaveButtonClicked);
    connect(m_pRemoveButton,
            &QPushButton::clicked,
            this,
            &ColorPaletteEditor::slotRemoveButtonClicked);
}

void ColorPaletteEditor::initialize(
        UserSettingsPointer pConfig,
        const QString& paletteName) {
    DEBUG_ASSERT(!m_pConfig);
    m_pConfig = pConfig;
    m_resetPalette = paletteName;
    QString saveName = paletteName;

    for (const ColorPalette& palette : mixxx::PredefinedColorPalettes::kPalettes) {
        if (paletteName == palette.getName()) {
            saveName = paletteName + QStringLiteral(" (") + tr("Edited") + QChar(')');
            ColorPaletteSettings colorPaletteSettings(m_pConfig);
            if (colorPaletteSettings.getColorPaletteNames().contains(saveName)) {
                m_resetPalette = saveName;
            }
            break;
        }
    }

    m_pSaveAsEdit->setText(saveName);

    slotResetButtonClicked();
}

void ColorPaletteEditor::slotUpdateButtons() {
    bool bDirty = m_pModel->isDirty();
    bool bEmpty = m_pModel->isEmpty();
    m_pSaveButton->setEnabled(
            !m_pSaveAsEdit->text().trimmed().isEmpty() &&
            (!m_bPaletteExists || (!m_bPaletteIsReadOnly && bDirty && !bEmpty)));
    m_pRemoveButton->setEnabled(
            m_bPaletteExists &&
            !m_bPaletteIsReadOnly);
    m_pResetButton->setEnabled(bDirty);
}

void ColorPaletteEditor::slotTableViewDoubleClicked(const QModelIndex& index) {
    if (index.isValid() && index.column() == 0) {
        QStandardItem* pColorItem = m_pModel->item(index.row(), 0);
        QColor oldColor = QColor(pColorItem->text());
        QColor newColor = QColorDialog::getColor(oldColor);
        if (newColor.isValid() && oldColor != newColor) {
            m_pModel->setColor(index.row(), newColor);
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
                m_pModel->setColorPalette(colorPaletteSettings.getColorPalette(
                        text, mixxx::PredefinedColorPalettes::kDefaultHotcueColorPalette));
            }
        }
    }

    m_bPaletteExists = bPaletteExists;
    m_bPaletteIsReadOnly = bPaletteIsReadOnly;

    slotUpdateButtons();
}

void ColorPaletteEditor::slotCloseButtonClicked() {
    reject();
}

void ColorPaletteEditor::slotRemoveButtonClicked() {
    QString paletteName = m_pSaveAsEdit->text().trimmed();
    ColorPaletteSettings colorPaletteSettings(m_pConfig);
    colorPaletteSettings.removePalette(paletteName);
    emit paletteRemoved(paletteName);
    accept();
}

void ColorPaletteEditor::slotSaveButtonClicked() {
    QString paletteName = m_pSaveAsEdit->text().trimmed();
    ColorPaletteSettings colorPaletteSettings(m_pConfig);
    colorPaletteSettings.setColorPalette(paletteName, m_pModel->getColorPalette(paletteName));
    m_pModel->setDirty(false);
    emit paletteChanged(paletteName);
    accept();
}

void ColorPaletteEditor::slotResetButtonClicked() {
    ColorPaletteSettings colorPaletteSettings(m_pConfig);
    ColorPalette palette = colorPaletteSettings.getColorPalette(
            m_resetPalette,
            mixxx::PredefinedColorPalettes::kDefaultHotcueColorPalette);
    m_pModel->setColorPalette(palette);
    slotUpdateButtons();
}
