#include <QPainter>
#include <QFont>
#include <QDebug>

#include "bpmdelegate.h"

BPMDelegate::BPMDelegate(QObject *parent, int column, int columnLock)
                     : QStyledItemDelegate(parent) {
    if (QTableView *tableView = qobject_cast<QTableView *>(parent)) {
        m_pTableView = tableView;

        m_pWidget = new QWidget(m_pTableView);

        m_pBPM = new QDoubleSpinBox(m_pWidget);
        m_pBPM->setMinimumWidth(1);
        m_pBPM->setDecimals(2);
        m_pButton = new BPMButton(m_pWidget);
        m_pButton->setMinimumWidth(1);
        m_pButton->setMaximumWidth(20);

        m_pLayout = new QHBoxLayout;
        m_pLayout->addWidget(m_pButton);
        m_pLayout->addWidget(m_pBPM);
        m_pLayout->setContentsMargins(0,0,0,0);
        m_pLayout->setSpacing(0);

        m_pWidget->setLayout(m_pLayout);
        m_pWidget->hide();

        connect(m_pTableView, SIGNAL(entered(QModelIndex)),
                this, SLOT(cellEntered(QModelIndex)));
        m_isOneCellInEditMode = false;
        m_column=column;
        m_columnLock= columnLock;
    }
}

BPMDelegate::~BPMDelegate() {
}

QWidget * BPMDelegate::createEditor(QWidget *parent,
                                              const QStyleOptionViewItem &option,
                                              const QModelIndex &index) const {
    // Populate the correct colors based on the styling
    QStyleOptionViewItem newOption = option;
    initStyleOption(&newOption, index);

    QWidget *pw = new QWidget(parent);
    BPMButton * btn = new BPMButton(pw);
    btn->setObjectName("oooo");
    connect(btn, SIGNAL(released()),
            this, SLOT(commitAndCloseEditor()));
    btn->setMaximumWidth(20);
    btn->setChecked(index.sibling(index.row(),m_columnLock).data().toBool());
    QDoubleSpinBox *pSpin = new QDoubleSpinBox(pw);
    pSpin->setValue(index.data().toDouble());pSpin->setDecimals(10);
    pSpin->setMinimum(0);pSpin->setMaximum(1000);
    pSpin->setSingleStep(0.1);
    QHBoxLayout *pLayout = new QHBoxLayout;
    pLayout->addWidget(btn);
    pLayout->addWidget(pSpin);
    pLayout->setContentsMargins(0,0,0,0);
    pLayout->setSpacing(0);
    pw->setLayout(pLayout);
    return pw;
}

void BPMDelegate::setEditorData(QWidget *editor,
                                          const QModelIndex &index) const {
    Q_UNUSED(editor);
    Q_UNUSED(index);
}

void BPMDelegate::setModelData(QWidget *editor,
                                         QAbstractItemModel *model,
                                         const QModelIndex &index) const {
    Q_UNUSED(editor);
    Q_UNUSED(model);
    Q_UNUSED(index);
}

void BPMDelegate::paint(QPainter *painter,
                                  const QStyleOptionViewItem &option,
                                  const QModelIndex &index) const {
    Q_UNUSED(index);
    m_pWidget->setGeometry(option.rect);
    m_pBPM->setValue(index.data().toDouble());
    m_pButton->setChecked(index.sibling(index.row(),m_columnLock).data().toBool());
    if (option.state == QStyle::State_Selected)
        painter->fillRect(option.rect, option.palette.base());
    QPixmap map = QPixmap::grabWidget(m_pWidget);
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
    Q_UNUSED(option);
    Q_UNUSED(index);
    return m_pWidget->sizeHint();
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
    if (BPMButton *editor = qobject_cast<BPMButton *>(sender())) {
        qDebug() << editor->isChecked();
        
    }
    // emit commitData(sender());
    // emit closeEditor(sender());
}
