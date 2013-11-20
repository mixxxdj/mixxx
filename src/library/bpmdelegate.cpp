#include "bpmdelegate.h"

BPMDelegate::BPMDelegate(QObject *parent, int column, int columnLock)
            : QStyledItemDelegate(parent),
              m_pEditor(new BPMEditor(BPMEditor::ReadOnly,
                        qobject_cast<QWidget *>(parent))),
              m_column(column),
              m_columnLock(columnLock) {
    m_pEditor->hide();
}

BPMDelegate::~BPMDelegate() {
}

QWidget* BPMDelegate::createEditor(QWidget *parent,
                                   const QStyleOptionViewItem &option,
                                   const QModelIndex &index) const {
    // Populate the correct colors based on the styling
    QStyleOptionViewItem newOption = option;
    initStyleOption(&newOption, index);

    BPMEditor *pEditor = new BPMEditor(BPMEditor::Editable,parent);
    pEditor->setPalette(option.palette);
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

void BPMDelegate::paint(QPainter *painter,const QStyleOptionViewItem &option,
                        const QModelIndex &index) const {

    m_pEditor->setData(index,m_columnLock);
    m_pEditor->setPalette(option.palette);
    m_pEditor->setGeometry(option.rect);
    if (option.state == QStyle::State_Selected) {
        painter->fillRect(option.rect, option.palette.base());
    }
    painter->save();
    painter->translate(option.rect.topLeft());
    m_pEditor->render(painter);
    painter->restore();
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
    Q_UNUSED(option);
    return m_pEditor->sizeHint();
}

void BPMDelegate::commitAndCloseEditor() {
    BPMEditor *editor = qobject_cast<BPMEditor *>(sender());
    emit commitData(editor);
    emit closeEditor(editor);
}
