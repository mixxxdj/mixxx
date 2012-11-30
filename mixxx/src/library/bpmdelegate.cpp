#include "bpmdelegate.h"
#include "bpmeditor.h"

BPMDelegate::BPMDelegate(QObject *parent, int column, int columnLock)
                     : QStyledItemDelegate(parent),
                       m_column(column),
                       m_columnLock(columnLock){
    m_pTableView = qobject_cast<QTableView *>(parent);
}

BPMDelegate::~BPMDelegate() {
}

QWidget * BPMDelegate::createEditor(QWidget *parent,
                                    const QStyleOptionViewItem &option,
                                    const QModelIndex &index) const {
    // Populate the correct colors based on the styling
    QStyleOptionViewItem newOption = option;
    initStyleOption(&newOption, index);

    BPMEditor *pEditor = new BPMEditor(newOption,BPMEditor::Editable,parent);
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
            m_currentEditedCellIndex = QModelIndex();
        }
    }
}

void BPMDelegate::commitAndCloseEditor() {
    BPMEditor *editor = qobject_cast<BPMEditor *>(sender());
    emit commitData(editor);
    emit closeEditor(editor);
}
