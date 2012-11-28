#include <QPainter>
#include <QFont>
#include <QDebug>

#include "bpmdelegate.h"
#include "bpmeditor.h"

BPMDelegate::BPMDelegate(QObject *parent, int columnLock)
                     : QStyledItemDelegate(parent) {
    m_pTableView = qobject_cast<QTableView *>(parent);
    connect(m_pTableView, SIGNAL(entered(QModelIndex)),
            this, SLOT(cellEntered(QModelIndex)));
    m_columnLock= columnLock;
}

BPMDelegate::~BPMDelegate() {
}

QWidget * BPMDelegate::createEditor(QWidget *parent,
                                    const QStyleOptionViewItem &option,
                                    const QModelIndex &index) const {
    // Populate the correct colors based on the styling
    QStyleOptionViewItem newOption = option;
    initStyleOption(&newOption, index);

    BPMEditor *pEditor = new BPMEditor(newOption,BPMEditor::Editable,m_pTableView);
    connect(pEditor, SIGNAL(finishedEditing()),
            this, SLOT(commitAndCloseEditor()));
    return pEditor;
}

void BPMDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const {
    BPMEditor *pEditor = qobject_cast<BPMEditor *>(editor);
    pEditor->setData(index,m_columnLock);
}

void BPMDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                               const QModelIndex &index) const {
    BPMEditor *pEditor = qobject_cast<BPMEditor *>(editor);
    model->setData(index,qVariantFromValue(pEditor->getBPM()));
    model->setData(index.sibling(index.row(),m_columnLock),
                   qVariantFromValue(pEditor->getLock()));
}

void BPMDelegate::paint(QPainter *painter,
                                  const QStyleOptionViewItem &option,
                                  const QModelIndex &index) const {
    // Let the editor paint in this case
    if (index==m_currentEditedCellIndex)
        return;

    // Populate the correct colors based on the styling
    QStyleOptionViewItem newOption = option;
    initStyleOption(&newOption, index);

    // Set the palette appropriately based on whether the row is selected or
    // not. We also have to check if it is inactive or not and use the
    // appropriate ColorGroup.
    if (newOption.state & QStyle::State_Selected) {
        QPalette::ColorGroup colorGroup =
                newOption.state & QStyle::State_Active ?
                QPalette::Active : QPalette::Inactive;
        painter->fillRect(newOption.rect,
            newOption.palette.color(colorGroup, QPalette::Highlight));
        painter->setBrush(newOption.palette.color(
            colorGroup, QPalette::HighlightedText));
    } else {
        painter->fillRect(newOption.rect, newOption.palette.base());
        painter->setBrush(newOption.palette.text());
    }

    BPMEditor editor(option,BPMEditor::ReadOnly, m_pTableView);
    editor.setData(index,m_columnLock);
    editor.setGeometry(option.rect);
    if (option.state == QStyle::State_Selected)
        painter->fillRect(option.rect, option.palette.base());
    QPixmap map = QPixmap::grabWidget(&editor);
    painter->drawPixmap(option.rect.x(),option.rect.y(),map);
}

void BPMDelegate::updateEditorGeometry(QWidget *editor,
                                       const QStyleOptionViewItem &option,
                                       const QModelIndex &index) const {
    Q_UNUSED(index);
    editor->setGeometry(option.rect);
}

QSize BPMDelegate::sizeHint(const QStyleOptionViewItem &option,
                            const QModelIndex &index) const {
    Q_UNUSED(index);
    BPMEditor editor(option,BPMEditor::ReadOnly,m_pTableView);
    return editor.sizeHint();
}

void BPMDelegate::cellEntered(const QModelIndex &index) {
    //this slot is called if the mouse pointer enters ANY cell on
    //the QTableView but the code should only be executed on a button
    if (index.column()==m_column) {
        if (m_isOneCellInEditMode) {
            m_pTableView->closePersistentEditor(m_currentEditedCellIndex);
        }
        m_pTableView->openPersistentEditor(index);
        m_isOneCellInEditMode = true;
        m_currentEditedCellIndex = index;
    } else { // close editor if the mouse leaves the button
        if (m_isOneCellInEditMode) {
            m_isOneCellInEditMode = false;
            m_pTableView->closePersistentEditor(m_currentEditedCellIndex);
        }
    }
}

void BPMDelegate::commitAndCloseEditor() {
    BPMEditor *editor = qobject_cast<BPMEditor *>(sender());
    emit commitData(editor);
    emit closeEditor(editor);
}
