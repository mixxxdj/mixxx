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
        : QWidget(parent),
          m_bPaletteExists(false),
          m_bPaletteIsReadOnly(false),
          m_pPaletteTemplateComboBox(make_parented<QComboBox>()),
          m_pSaveAsComboBox(make_parented<QComboBox>()),
          m_pTableView(make_parented<QTableView>()),
          m_pModel(make_parented<ColorPaletteEditorModel>(m_pTableView)) {
    m_pSaveAsComboBox->setEditable(true);

    m_pResetButton = make_parented<QPushButton>(tr("Reset"), this);

    QDialogButtonBox* pButtonBox = new QDialogButtonBox();
    m_pRemoveButton = pButtonBox->addButton(
            tr("Remove Palette"),
            QDialogButtonBox::DestructiveRole);
    m_pSaveButton = pButtonBox->addButton(QDialogButtonBox::Save);
    m_pCloseButton = pButtonBox->addButton(QDialogButtonBox::Close);

    QHBoxLayout* pTopLayout = new QHBoxLayout();
    pTopLayout->addWidget(new QLabel(tr("Name")));
    pTopLayout->addWidget(m_pSaveAsComboBox, 1);

    QHBoxLayout* pBottomLayout = new QHBoxLayout();
    pBottomLayout->addWidget(new QLabel(tr("Reset to")));
    pBottomLayout->addWidget(m_pPaletteTemplateComboBox, 1);
    pBottomLayout->addWidget(m_pResetButton.get());

    QVBoxLayout* pLayout = new QVBoxLayout();
    pLayout->addLayout(pTopLayout);
    pLayout->addWidget(m_pTableView, 1);
    pLayout->addLayout(pBottomLayout);
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
    connect(m_pSaveAsComboBox,
            &QComboBox::editTextChanged,
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

void ColorPaletteEditor::initialize(UserSettingsPointer pConfig) {
    DEBUG_ASSERT(!m_pConfig);
    m_pConfig = pConfig;
    reset();
}

void ColorPaletteEditor::reset() {
    m_pPaletteTemplateComboBox->clear();
    m_pSaveAsComboBox->clear();

    for (const ColorPalette& palette : mixxx::PredefinedColorPalettes::kPalettes) {
        m_pPaletteTemplateComboBox->addItem(palette.getName());
    }

    ColorPaletteSettings colorPaletteSettings(m_pConfig);
    if (colorPaletteSettings.getColorPaletteNames().count()) {
        for (const QString& paletteName : colorPaletteSettings.getColorPaletteNames()) {
            m_pSaveAsComboBox->addItem(paletteName);
            m_pPaletteTemplateComboBox->addItem(paletteName);
        }
    } else {
        m_pSaveAsComboBox->addItem(tr("Custom Color Palette"));
        slotResetButtonClicked();
    }
}

void ColorPaletteEditor::slotUpdateButtons() {
    bool bDirty = m_pModel->isDirty();
    bool bEmpty = m_pModel->isEmpty();
    m_pSaveButton->setEnabled(
            !m_pSaveAsComboBox->currentText().isEmpty() &&
            (!m_bPaletteExists || (!m_bPaletteIsReadOnly && bDirty && !bEmpty)));
    m_pRemoveButton->setEnabled(
            m_bPaletteExists &&
            !m_bPaletteIsReadOnly);
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

void ColorPaletteEditor::slotCloseButtonClicked() {
    if (m_pSaveButton->isEnabled()) {
        QMessageBox msgBox;
        msgBox.setWindowTitle(tr("Custom Palettes Editor"));
        msgBox.setText(tr(
                "The custom palette is not saved.\n"
                "Close anyway?"));
        msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Cancel);
        int ret = msgBox.exec();
        if (ret == QMessageBox::Ok) {
            emit closeButtonClicked();
        }
    } else {
        emit closeButtonClicked();
    }
}

void ColorPaletteEditor::slotRemoveButtonClicked() {
    QString paletteName = m_pSaveAsComboBox->currentText();
    ColorPaletteSettings colorPaletteSettings(m_pConfig);
    colorPaletteSettings.removePalette(paletteName);
    reset();
    emit paletteRemoved(paletteName);
}

void ColorPaletteEditor::slotSaveButtonClicked() {
    QString paletteName = m_pSaveAsComboBox->currentText();
    ColorPaletteSettings colorPaletteSettings(m_pConfig);
    colorPaletteSettings.setColorPalette(paletteName, m_pModel->getColorPalette(paletteName));
    m_pModel->setDirty(false);
    reset();
    m_pSaveAsComboBox->setCurrentText(paletteName);
    emit paletteChanged(paletteName);
}

void ColorPaletteEditor::slotResetButtonClicked() {
    QString paletteName = m_pPaletteTemplateComboBox->currentText();
    ColorPaletteSettings colorPaletteSettings(m_pConfig);
    ColorPalette palette = colorPaletteSettings.getColorPalette(
            paletteName,
            mixxx::PredefinedColorPalettes::kDefaultHotcueColorPalette);
    m_pModel->setColorPalette(palette);
    m_pModel->setDirty(true);
    slotUpdateButtons();
}
