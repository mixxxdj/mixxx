#include <QPainter>
#include <QFont>
#include <QDebug>

#include "bpmdelegate.h"

BPMDelegate::BPMDelegate(QObject *parent, int column)
                     : QStyledItemDelegate(parent) {
    if (QTableView *tableView = qobject_cast<QTableView *>(parent)) {
        m_pTableView = tableView;

        m_pWidget = new QWidget(m_pTableView);

        m_pBPM = new QDoubleSpinBox(m_pWidget);
        m_pBPM->setMinimumWidth(1);
        m_pBPM->setDecimals(2);
        m_pButton = new BPMButton(m_pWidget);
        m_pButton->setMinimumWidth(1);

        m_pLayout = new QHBoxLayout;
        m_pLayout->addWidget(m_pButton);
        m_pLayout->addSpacing(2);
        m_pLayout->addWidget(m_pBPM);
        m_pLayout->setContentsMargins(0,0,0,0);
        m_pLayout->setSpacing(0);

        m_pWidget->setLayout(m_pLayout);
        m_pWidget->hide();

        connect(m_pTableView, SIGNAL(entered(QModelIndex)),
                this, SLOT(cellEntered(QModelIndex)));
        m_isOneCellInEditMode = false;
        m_column=column;
    }
}

BPMDelegate::~BPMDelegate() {
}

QWidget * BPMDelegate::createEditor(QWidget *parent,
                                              const QStyleOptionViewItem &option,
                                              const QModelIndex &index) const {
    Q_UNUSED(option);
    Q_UNUSED(index);
    QWidget *pw = new QWidget(parent);
    QPushButton * btn = new QPushButton(pw);
    btn->setIcon(QIcon("res/images/library/uncheck.png"));
    btn->setObjectName("oooo");
    btn->setMaximumWidth(30);
    QDoubleSpinBox *pSpin = new QDoubleSpinBox(pw);
    pSpin->setValue(index.data().toDouble());pSpin->setDecimals(10);
    pSpin->setMinimum(0);pSpin->setMaximum(1000);
    pSpin->setSingleStep(0.1);
    QHBoxLayout *pLayout = new QHBoxLayout;
    pLayout->addWidget(btn);pLayout->addSpacing(2);
    pLayout->addWidget(pSpin);pLayout->setContentsMargins(0,0,0,0);
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
