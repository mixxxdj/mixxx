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

#include "moc_colorpaletteeditor.cpp"
#include "preferences/colorpalettesettings.h"
#include "util/color/predefinedcolorpalettes.h"

namespace {
const QColor kDefaultPaletteColor(0, 0, 0);
} // namespace

ColorPaletteEditor::ColorPaletteEditor(QWidget* parent, bool showHotcueNumbers)
        : QDialog(parent),
          m_bPaletteExists(false),
          m_bPaletteIsReadOnly(false),
          m_pSaveAsEdit(make_parented<QLineEdit>(this)),
          m_pTableView(make_parented<QTableView>(this)),
          m_pModel(make_parented<ColorPaletteEditorModel>(m_pTableView)) {
    // Create widgets
    QHBoxLayout* pColorButtonLayout = new QHBoxLayout();
    QWidget* pExpander = new QWidget(this);
    pExpander->setSizePolicy(
            QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred));
    pColorButtonLayout->addWidget(pExpander);

    m_pRemoveColorButton = new QPushButton(QIcon::fromTheme("list-remove"), "", this);
    if (m_pRemoveColorButton->icon().isNull()) {
        m_pRemoveColorButton->setText("-");
    }
    m_pRemoveColorButton->setFixedWidth(32);
    m_pRemoveColorButton->setToolTip(tr("Remove Color"));
    m_pRemoveColorButton->setDisabled(true);
    pColorButtonLayout->addWidget(m_pRemoveColorButton);
    connect(m_pRemoveColorButton,
            &QPushButton::clicked,
            this,
            &ColorPaletteEditor::slotRemoveColor);

    m_pAddColorButton = new QPushButton(QIcon::fromTheme("list-add"), "", this);
    if (m_pAddColorButton->icon().isNull()) {
        m_pAddColorButton->setText("+");
    }
    m_pAddColorButton->setFixedWidth(32);
    m_pAddColorButton->setToolTip(tr("Add Color"));
    pColorButtonLayout->addWidget(m_pAddColorButton);
    connect(m_pAddColorButton,
            &QPushButton::clicked,
            this,
            &ColorPaletteEditor::slotAddColor);

    QHBoxLayout* pNameLayout = new QHBoxLayout();
    pNameLayout->addWidget(new QLabel(tr("Name")));
    pNameLayout->addWidget(m_pSaveAsEdit, 1);

    QDialogButtonBox* pPaletteButtonBox = new QDialogButtonBox();
    m_pResetButton = pPaletteButtonBox->addButton(QDialogButtonBox::Reset);
    m_pRemoveButton = pPaletteButtonBox->addButton(
            tr("Remove Palette"),
            QDialogButtonBox::ResetRole);
    m_pCloseButton = pPaletteButtonBox->addButton(QDialogButtonBox::Discard);
    m_pSaveButton = pPaletteButtonBox->addButton(QDialogButtonBox::Ok);

    // Add widgets to dialog
    QVBoxLayout* pLayout = new QVBoxLayout();
    pLayout->addWidget(m_pTableView, 1);
    pLayout->addLayout(pColorButtonLayout);
    pLayout->addLayout(pNameLayout);
    pLayout->addWidget(pPaletteButtonBox);
    setLayout(pLayout);
    setContentsMargins(0, 0, 0, 0);

    // Set up model
    m_pModel->setColumnCount(2);
    m_pModel->setHeaderData(0, Qt::Horizontal, tr("Color"), Qt::DisplayRole);
    m_pModel->setHeaderData(1, Qt::Horizontal, tr("Assign to Hotcue Number"), Qt::DisplayRole);
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
    m_pTableView->horizontalHeader()->setStretchLastSection(true);

    if (!showHotcueNumbers) {
        m_pTableView->hideColumn(1);
    }

    connect(m_pTableView,
            &QTableView::doubleClicked,
            this,
            &ColorPaletteEditor::slotTableViewDoubleClicked);
    connect(m_pTableView->selectionModel(),
            &QItemSelectionModel::selectionChanged,
            this,
            &ColorPaletteEditor::slotSelectionChanged);
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

void ColorPaletteEditor::slotAddColor() {
    m_pModel->appendRow(kDefaultPaletteColor);
    m_pTableView->scrollToBottom();
    m_pTableView->setCurrentIndex(
            m_pModel->index(m_pModel->rowCount() - 1, 0));
}

void ColorPaletteEditor::slotRemoveColor() {
    QModelIndexList selection = m_pTableView->selectionModel()->selectedRows();
    for (const auto& index : selection) {
        //row selected
        int row = index.row();
        m_pModel->removeRow(row);
    }
    m_pRemoveColorButton->setDisabled(
            !m_pTableView->selectionModel()->hasSelection());
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

    QMessageBox msgBox;
    msgBox.setWindowTitle(tr("Remove Palette"));
    msgBox.setText(tr(
            "Do you really want to remove the palette permanently?"));
    msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Cancel);
    int ret = msgBox.exec();
    if (ret == QMessageBox::Ok) {
        ColorPaletteSettings colorPaletteSettings(m_pConfig);
        colorPaletteSettings.removePalette(paletteName);
        emit paletteRemoved(paletteName);
        accept();
    }
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

void ColorPaletteEditor::slotSelectionChanged(
        const QItemSelection& selected,
        const QItemSelection& deselected) {
    Q_UNUSED(deselected);
    m_pRemoveColorButton->setDisabled(!selected.count());
}
